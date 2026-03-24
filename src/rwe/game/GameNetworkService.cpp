#include "GameNetworkService.h"
#include <algorithm>
#include <rwe/network_util.h>
#include <rwe/proto/serialization.h>
#include <rwe/sim/GameHash.h>
#include <rwe/sim/SimTicksPerSecond.h>
#include <rwe/util/Index.h>
#include <rwe/util/OpaqueId_io.h>
#include <rwe/util/range_util.h>
#include <rwe/util/SimpleLogger.h>
#include <thread>

namespace rwe
{
    GameNetworkService::GameNetworkService(
        PlayerId localPlayerId,
        int port,
        const std::vector<GameNetworkService::EndpointInfo>& endpoints,
        PlayerCommandService* playerCommandService)
        : localPlayerId(localPlayerId),
          port(port),
          resolver(ioContext),
          socket(ioContext),
          sendTimer(ioContext),
          endpoints(endpoints),
          playerCommandService(playerCommandService)
    {
    }

    GameNetworkService::~GameNetworkService()
    {
        if (networkThread.joinable())
        {
            ioContext.stop();
            networkThread.join();
        }
    }

    void GameNetworkService::start()
    {
        networkThread = std::thread(&GameNetworkService::run, this);
    }

    void GameNetworkService::submitCommands(SceneTime currentSceneTime, const GameNetworkService::CommandSet& commands)
    {
        boost::asio::post(ioContext,[this, currentSceneTime, commands]() {
            this->currentSceneTime = currentSceneTime;
            for (auto& e : endpoints)
            {
                e.sendBuffer.push_back(commands);
            }
        });
    }

    void GameNetworkService::submitGameHash(GameHash hash)
    {
        boost::asio::post(ioContext,[this, hash]() {
            for (auto& e : endpoints)
            {
                e.hashSendBuffer.push_back(hash);
            }
        });
    }

    SceneTime GameNetworkService::estimateAvergeSceneTime(SceneTime localSceneTime)
    {
        std::promise<unsigned int> result;
        boost::asio::post(ioContext,[this, localSceneTime, &result]() {
            auto time = getTimestamp();
            auto otherTimes = choose(endpoints, [](const auto& e) { return e.lastKnownSceneTime; });

            auto finalValue = estimateAverageSceneTimeStatic(localSceneTime, otherTimes, time);
            result.set_value(finalValue);
        });

        return SceneTime(result.get_future().get());
    }

    float GameNetworkService::getMaxAverageRttMillis()
    {
        std::promise<float> result;
        boost::asio::post(ioContext,[this, &result]() {
            auto maxRtt = 0.0f;
            for (const auto& e : endpoints)
            {
                if (e.averageRoundTripTime > maxRtt)
                {
                    maxRtt = e.averageRoundTripTime;
                }
            }

            result.set_value(maxRtt);
        });

        return result.get_future().get();
    }

    void GameNetworkService::run()
    {
        try
        {
            auto endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v6(), port);
            socket.open(endpoint.protocol());
            socket.bind(endpoint);

            listenForNextMessage();

            sendLoop();

            ioContext.run();
        }
        catch (const std::exception& e)
        {
            LOG_ERROR << "Network thread died with error: " << e.what();
        }
    }

    void GameNetworkService::listenForNextMessage()
    {
        socket.async_receive_from(
            boost::asio::buffer(receiveBuffer.data(), receiveBuffer.size()),
            currentRemoteEndpoint,
            [this](const auto& error, const auto& bytesTransferred) {
                receive(error, bytesTransferred);
                listenForNextMessage();
            });
    }

    proto::NetworkMessage createProtoMessage(
        int packetId,
        PlayerId playerId,
        SceneTime currentSceneTime,
        SequenceNumber nextCommandToSend,
        SequenceNumber nextCommandToReceive,
        GameTime nextHashToSend,
        GameTime nextHashToReceive,
        std::chrono::milliseconds ackDelay,
        const std::deque<GameNetworkService::CommandSet>& sendBuffer,
        const std::deque<GameHash>& gameHashBuffer)
    {
        proto::NetworkMessage outerMessage;
        auto& m = *outerMessage.mutable_game_update();
        m.set_packet_id(packetId);
        m.set_player_id(playerId.value);
        m.set_current_scene_time(currentSceneTime.value);
        m.set_next_command_set_to_send(nextCommandToSend.value);
        m.set_next_command_set_to_receive(nextCommandToReceive.value);
        m.set_next_game_hash_to_send(nextHashToSend.value);
        m.set_next_game_hash_to_receive(nextHashToReceive.value);
        m.set_ack_delay(ackDelay.count());

        for (const auto& set : sendBuffer)
        {
            auto& setMessage = *m.add_command_set();

            for (const auto& cmd : set)
            {
                auto& cmdMessage = *setMessage.add_command();
                serializePlayerCommand(cmd, cmdMessage);
            }
        }

        for (const auto& hash : gameHashBuffer)
        {
            m.add_game_hashes(hash.value);
        }

        return outerMessage;
    }

    void GameNetworkService::sendLoop()
    {
        sendToAll();
        sendTimer.expires_after(std::chrono::milliseconds(100));
        sendTimer.async_wait([this](const boost::system::error_code& error) {
            if (error)
            {
                LOG_ERROR << "Boost error while waiting on timer: " << error.message();
                return;
            }

            sendLoop();
        });
    }

    void GameNetworkService::sendToAll()
    {
        for (auto& e : endpoints)
        {
            send(e);
        }
    }

    void GameNetworkService::send(GameNetworkService::EndpointInfo& endpoint)
    {
        auto packetId = uniform_dist(gen);
        LOG_DEBUG << "Sending packet ID " << packetId << " to endpoint: " << endpoint.endpoint.address().to_string() << ":" << endpoint.endpoint.port();
        std::chrono::milliseconds delay(0);
        auto sendTime = getTimestamp();
        if (endpoint.lastReceiveTime)
        {
            delay = std::chrono::duration_cast<std::chrono::milliseconds>(sendTime - *endpoint.lastReceiveTime);
        }

        auto message = createProtoMessage(packetId, localPlayerId, currentSceneTime, endpoint.nextCommandToSend, endpoint.nextCommandToReceive, endpoint.nextHashToSend, endpoint.nextHashToReceive, delay, endpoint.sendBuffer, endpoint.hashSendBuffer);
        auto messageSize = message.ByteSizeLong();
        if (messageSize > getSize(sendBuffer) - 4)
        {
            throw std::runtime_error("Message to be sent was bigger than buffer size");
        }
        if (!message.SerializeToArray(sendBuffer.data(), sendBuffer.size()))
        {
            throw std::runtime_error("Failed to serialize message to buffer");
        }

        // throw in a CRC to verify the message
        writeInt(&sendBuffer[messageSize], computeCrc(sendBuffer.data(), messageSize));

        socket.send_to(boost::asio::buffer(sendBuffer.data(), messageSize + 4), endpoint.endpoint);

        auto nextSequenceNumber = SequenceNumber(endpoint.nextCommandToSend.value + (endpoint.sendBuffer.size()));
        if (endpoint.sendTimes.empty() || endpoint.sendTimes.back().first < nextSequenceNumber)
        {
            endpoint.sendTimes.emplace_back(nextSequenceNumber, sendTime);
        }
    }

    void GameNetworkService::receive(const boost::system::error_code& error, std::size_t receivedBytes)
    {
        if (error)
        {
            LOG_ERROR << "Boost error on receive: " << error.message();
            return;
        }

        auto receiveTime = getTimestamp();
        LOG_DEBUG << "Received " << receivedBytes << " bytes from endpoint: " << currentRemoteEndpoint.address().to_string() << ":" << currentRemoteEndpoint.port();

        if (receivedBytes == receiveBuffer.size())
        {
            LOG_WARN << "Received " << receivedBytes << " bytes, which filled the entire message buffer!!";
        }

        if (receivedBytes < 4)
        {
            LOG_ERROR << "Received message is too short (" << receivedBytes << " bytes), ignoring";
            return;
        }

        auto endpointIt = std::find_if(endpoints.begin(), endpoints.end(), [this](const auto& e) { return currentRemoteEndpoint == e.endpoint; });
        if (endpointIt == endpoints.end())
        {
            // message was from some unknown address, ignore it
            LOG_DEBUG << "Unknown address, ignoring";
            return;
        }

        auto receivedCrc = readInt(&receiveBuffer[receivedBytes - 4]);
        auto computedCrc = computeCrc(receiveBuffer.data(), receivedBytes - 4);
        if (receivedCrc != computedCrc)
        {
            LOG_ERROR << "Message CRC incorrect, ignoring";
            return;
        }

        proto::NetworkMessage outerMessage;
        outerMessage.ParseFromArray(receiveBuffer.data(), receivedBytes - 4);
        if (!outerMessage.has_game_update())
        {
            // message wasn't a game update, ignore it
            LOG_DEBUG << "Not game update, ignoring";
            return;
        }

        EndpointInfo& endpoint = *endpointIt;

        const auto& message = outerMessage.game_update();

        LOG_DEBUG << "Packet received with ID " << message.packet_id();
        if (message.player_id() != endpoint.playerId.value)
        {
            LOG_ERROR << "Player " << endpoint.playerId.value << " endpoint sent wrong player ID: " << message.player_id();
            return;
        }

        LOG_DEBUG << "Received ack to " << message.next_command_set_to_receive() << " and " << message.command_set_size() << " commands starting at " << message.next_command_set_to_send();

        SequenceNumber newNextCommandToSend(message.next_command_set_to_receive());
        if (newNextCommandToSend.value > endpoint.nextCommandToSend.value + endpoint.sendBuffer.size())
        {
            LOG_ERROR << "Remote acked up to " << newNextCommandToSend.value << ", but we are at " << endpoint.nextCommandToSend.value << " and command buffer contains " << endpoint.sendBuffer.size() << " elements";
        }
        while (newNextCommandToSend > endpoint.nextCommandToSend && !endpoint.sendBuffer.empty())
        {
            endpoint.sendBuffer.pop_front();
            endpoint.nextCommandToSend = SequenceNumber(endpoint.nextCommandToSend.value + 1);
        }

        while (!endpoint.sendTimes.empty() && endpoint.nextCommandToSend > endpoint.sendTimes.front().first)
        {
            // skip older send time measurements
            endpoint.sendTimes.pop_front();
        }
        if (!endpoint.sendTimes.empty() && endpoint.nextCommandToSend == endpoint.sendTimes.front().first)
        {
            auto roundTripTime = receiveTime - endpoint.sendTimes.front().second;
            auto ackDelay = std::chrono::milliseconds(message.ack_delay());
            roundTripTime = roundTripTime > ackDelay ? roundTripTime - ackDelay : std::chrono::milliseconds(0);
            auto rttMillis = std::chrono::duration_cast<std::chrono::milliseconds>(roundTripTime).count();
            endpoint.averageRoundTripTime = ema(rttMillis, endpoint.averageRoundTripTime, 0.1f);
            LOG_DEBUG << "Average RTT: " << endpoint.averageRoundTripTime << "ms";
        }

        auto extraFrames = static_cast<unsigned int>((endpoint.averageRoundTripTime / 2.0f) * SimTicksPerSecond / 1000.0f);
        endpoint.lastKnownSceneTime = std::make_pair(SceneTime(message.current_scene_time() + extraFrames), receiveTime);
        LOG_DEBUG << "Estimated peer scene time: " << endpoint.lastKnownSceneTime->first.value;

        SequenceNumber firstCommandNumber(message.next_command_set_to_send());
        if (firstCommandNumber > endpoint.nextCommandToReceive)
        {
            // message starts with commands too far in the future, ignore it.
            // FIXME: this should probably be an error as it shouldn't ever happen
            LOG_ERROR << "First command number in message was too high! Expecting no more than " << endpoint.nextCommandToReceive.value << ", received " << firstCommandNumber.value;
            return;
        }

        auto firstRelevantCommandIndex = (endpoint.nextCommandToReceive - firstCommandNumber).value;

        // if the packet is relevant (contains new information), process it
        if (firstRelevantCommandIndex < static_cast<unsigned int>(message.command_set_size()))
        {
            endpoint.lastReceiveTime = receiveTime;

            for (int i = firstRelevantCommandIndex; i < message.command_set_size(); ++i)
            {
                auto commandSet = deserializeCommandSet(message.command_set(i));
                playerCommandService->pushCommands(endpoint.playerId, commandSet);
                endpoint.nextCommandToReceive = SequenceNumber(endpoint.nextCommandToReceive.value + 1);
            }
        }

        GameTime newNextHashToSend(message.next_game_hash_to_receive());
        if (newNextHashToSend > endpoint.nextHashToSend + GameTime(endpoint.hashSendBuffer.size()))
        {
            LOG_ERROR << "Remote acked up to " << newNextHashToSend.value << ", but we are at " << endpoint.nextHashToSend.value << " and hash buffer contains " << endpoint.hashSendBuffer.size() << " elements";
        }
        while (newNextHashToSend > endpoint.nextHashToSend && !endpoint.hashSendBuffer.empty())
        {
            endpoint.hashSendBuffer.pop_front();
            endpoint.nextHashToSend += GameTime(1);
        }

        GameTime firstGameHashTime(message.next_game_hash_to_send());
        if (firstGameHashTime > endpoint.nextHashToReceive)
        {
            // message starts with hashes too far in the future, ignore it.
            // FIXME: this should probably be an error as it shouldn't ever happen
            LOG_ERROR << "First game hash time in message was too high! Expecting no more than " << endpoint.nextHashToReceive.value << ", received " << firstGameHashTime.value;
            return;
        }

        auto firstRelevantGameHashIndex = (endpoint.nextHashToReceive - firstGameHashTime).value;
        for (int i = firstRelevantGameHashIndex; i < message.game_hashes_size(); ++i)
        {
            playerCommandService->pushHash(endpoint.playerId, GameHash(message.game_hashes(i)));
            endpoint.nextHashToReceive += GameTime(1);
        }
    }
}
