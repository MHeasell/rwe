#include "GameNetworkService.h"
#include <boost/range/adaptors.hpp>
#include <rwe/GameHash.h>
#include <rwe/Index.h>
#include <rwe/OpaqueId_io.h>
#include <rwe/SceneManager.h>
#include <rwe/network_util.h>
#include <rwe/proto/serialization.h>
#include <rwe/range_util.h>
#include <rwe/sim/SimTicksPerSecond.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>
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
        ioContext.post([this, currentSceneTime, commands]() {
            this->currentSceneTime = currentSceneTime;
            for (auto& e : endpoints)
            {
                e.sendBuffer.push_back(commands);
            }
        });
    }

    void GameNetworkService::submitGameHash(GameHash hash)
    {
        ioContext.post([this, hash]() {
            for (auto& e : endpoints)
            {
                e.hashSendBuffer.push_back(hash);
            }
        });
    }

    SceneTime GameNetworkService::estimateAvergeSceneTime(SceneTime localSceneTime)
    {
        std::promise<unsigned int> result;
        ioContext.post([this, localSceneTime, &result]() {
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
        ioContext.post([this, &result]() {
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
            spdlog::get("rwe")->error("Network thread died with error: {0}", e.what());
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
        sendTimer.expires_from_now(std::chrono::milliseconds(100));
        sendTimer.async_wait([this](const boost::system::error_code& error) {
            if (error)
            {
                spdlog::get("rwe")->error("Boost error while waiting on timer: {}", error);
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
        spdlog::get("rwe")->debug("Sending packet ID {} to endpoint: {}:{}", packetId, endpoint.endpoint.address().to_string(), endpoint.endpoint.port());
        std::chrono::milliseconds delay(0);
        auto sendTime = getTimestamp();
        if (endpoint.lastReceiveTime)
        {
            delay = std::chrono::duration_cast<std::chrono::milliseconds>(sendTime - *endpoint.lastReceiveTime);
        }

        auto message = createProtoMessage(packetId, localPlayerId, currentSceneTime, endpoint.nextCommandToSend, endpoint.nextCommandToReceive, endpoint.nextHashToSend, endpoint.nextHashToReceive, delay, endpoint.sendBuffer, endpoint.hashSendBuffer);
        auto messageSize = message.ByteSize();
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
            spdlog::get("rwe")->error("Boost error on receive: {}", error);
            return;
        }

        auto receiveTime = getTimestamp();
        spdlog::get("rwe")->debug("Received {} bytes from endpoint: {}:{}", receivedBytes, currentRemoteEndpoint.address().to_string(), currentRemoteEndpoint.port());

        if (receivedBytes == receiveBuffer.size())
        {
            spdlog::get("rwe")->warn("Received {} bytes, which filled the entire message buffer!!", receivedBytes);
        }

        if (receivedBytes < 4)
        {
            spdlog::get("rwe")->error("Received message is too short, ignoring", receivedBytes);
            return;
        }

        auto endpointIt = std::find_if(endpoints.begin(), endpoints.end(), [this](const auto& e) { return currentRemoteEndpoint == e.endpoint; });
        if (endpointIt == endpoints.end())
        {
            // message was from some unknown address, ignore it
            spdlog::get("rwe")->debug("Unknown address, ignoring");
            return;
        }

        auto receivedCrc = readInt(&receiveBuffer[receivedBytes - 4]);
        auto computedCrc = computeCrc(receiveBuffer.data(), receivedBytes - 4);
        if (receivedCrc != computedCrc)
        {
            spdlog::get("rwe")->error("Message CRC incorrect, ignoring");
            return;
        }

        proto::NetworkMessage outerMessage;
        outerMessage.ParseFromArray(receiveBuffer.data(), receivedBytes - 4);
        if (!outerMessage.has_game_update())
        {
            // message wasn't a game update, ignore it
            spdlog::get("rwe")->debug("Not game update, ignoring");
            return;
        }

        EndpointInfo& endpoint = *endpointIt;

        const auto& message = outerMessage.game_update();

        spdlog::get("rwe")->debug("Packet received with ID {}", message.packet_id());
        if (message.player_id() != endpoint.playerId.value)
        {
            spdlog::get("rwe")->error("Player {} endpoint sent wrong player ID: {}", endpoint.playerId.value, message.player_id());
            return;
        }

        spdlog::get("rwe")->debug("Received ack to {0} and {1} commands starting at {2}", message.next_command_set_to_receive(), message.command_set_size(), message.next_command_set_to_send());

        SequenceNumber newNextCommandToSend(message.next_command_set_to_receive());
        if (newNextCommandToSend.value > endpoint.nextCommandToSend.value + endpoint.sendBuffer.size())
        {
            spdlog::get("rwe")->error(
                "Remote acked up to {0}, but we are at {1} and command buffer contains {2} elements",
                newNextCommandToSend,
                endpoint.nextCommandToSend,
                endpoint.sendBuffer.size());
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
            spdlog::get("rwe")->debug("Average RTT: {0}ms", endpoint.averageRoundTripTime);
        }

        auto extraFrames = static_cast<unsigned int>((endpoint.averageRoundTripTime / 2.0f) * SimTicksPerSecond / 1000.0f);
        endpoint.lastKnownSceneTime = std::make_pair(SceneTime(message.current_scene_time() + extraFrames), receiveTime);
        spdlog::get("rwe")->debug("Estimated peer scene time: {0}", endpoint.lastKnownSceneTime->first.value);

        SequenceNumber firstCommandNumber(message.next_command_set_to_send());
        if (firstCommandNumber > endpoint.nextCommandToReceive)
        {
            // message starts with commands too far in the future, ignore it.
            // FIXME: this should probably be an error as it shouldn't ever happen
            spdlog::get("rwe")->error("First command number in message was too high! Expecting no more than {0}, received {1}", endpoint.nextCommandToReceive.value, firstCommandNumber.value);
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
            spdlog::get("rwe")->error(
                "Remote acked up to {0}, but we are at {1} and hash buffer contains {2} elements",
                newNextHashToSend,
                endpoint.nextHashToSend,
                endpoint.hashSendBuffer.size());
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
            spdlog::get("rwe")->error("First game hash time in message was too high! Expecting no more than {0}, received {1}", endpoint.nextHashToReceive.value, firstGameHashTime.value);
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
