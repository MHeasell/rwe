#include "GameNetworkService.h"
#include "SceneManager.h"
#include <rwe/proto/serialization.h>
#include <spdlog/spdlog.h>
#include <thread>

namespace rwe
{
    GameNetworkService::GameNetworkService(
        const boost::asio::ip::udp::endpoint& localEndpoint,
        const std::vector<GameNetworkService::EndpointInfo>& endpoints,
        PlayerCommandService* playerCommandService)
        : localEndpoint(localEndpoint),
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

    SceneTime GameNetworkService::estimateAvergeSceneTime(SceneTime localSceneTime)
    {
        std::promise<unsigned int> result;
        ioContext.post([this, localSceneTime, &result]() {
            auto time = getTimestamp();

            auto accum = localSceneTime.value;
            for (const auto& e : endpoints)
            {
                if (!e.lastKnownSceneTime)
                {
                    continue;
                }

                auto elapsedTimeMillis = std::chrono::duration_cast<std::chrono::milliseconds>(time - e.lastKnownSceneTime->second).count();
                auto extraFrames = elapsedTimeMillis / 16;

                auto peerSceneTime = e.lastKnownSceneTime->first.value + extraFrames;
                accum += peerSceneTime;
            }
            auto finalValue = accum / (endpoints.size() + 1);

            result.set_value(finalValue);
        });

        return SceneTime(result.get_future().get());
    }

    void GameNetworkService::run()
    {
        try
        {
            socket.open(localEndpoint.protocol());
            socket.bind(localEndpoint);

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
            boost::asio::buffer(messageBuffer.data(), messageBuffer.size()),
            currentRemoteEndpoint,
            std::bind(&GameNetworkService::onReceive, this, std::placeholders::_1, std::placeholders::_2));
    }

    void GameNetworkService::onReceive(const boost::system::error_code& error, std::size_t bytesTransferred)
    {
        if (error)
        {
            // TODO: error handling here
            listenForNextMessage();
            return;
        }

        receive(bytesTransferred);

        listenForNextMessage();
    }

    proto::NetworkMessage createProtoMessage(
        SceneTime currentSceneTime,
        SequenceNumber nextCommandToSend,
        SequenceNumber nextCommandToReceive,
        std::chrono::milliseconds ackDelay,
        const std::deque<GameNetworkService::CommandSet>& sendBuffer)
    {
        proto::NetworkMessage outerMessage;
        auto& m = *outerMessage.mutable_game_update();
        m.set_current_scene_time(currentSceneTime.value);
        m.set_next_command_set_to_send(nextCommandToSend.value);
        m.set_next_command_set_to_receive(nextCommandToReceive.value);
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

        return outerMessage;
    }

    void GameNetworkService::sendLoop()
    {
        sendToAll();
        sendTimer.expires_from_now(std::chrono::milliseconds(100));
        sendTimer.async_wait([this](const boost::system::error_code& error) {
            if (error)
            {
                // TODO: log this
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
        spdlog::get("rwe")->debug("Sending to endpoint: {0}:{1}", endpoint.endpoint.address().to_string(), endpoint.endpoint.port());
        std::chrono::milliseconds delay(0);
        auto sendTime = getTimestamp();
        if (endpoint.lastReceiveTime)
        {
            delay = std::chrono::duration_cast<std::chrono::milliseconds>(sendTime - *endpoint.lastReceiveTime);
        }

        auto message = createProtoMessage(currentSceneTime, endpoint.nextCommandToSend, endpoint.nextCommandToReceive, delay, endpoint.sendBuffer);
        message.SerializeToArray(messageBuffer.data(), messageBuffer.size());
        socket.send_to(boost::asio::buffer(messageBuffer.data(), message.ByteSize()), endpoint.endpoint);

        auto nextSequenceNumber = SequenceNumber(endpoint.nextCommandToSend.value + (endpoint.sendBuffer.size()));
        if (endpoint.sendTimes.empty() || endpoint.sendTimes.front().first < nextSequenceNumber)
        {
            endpoint.sendTimes.emplace_back(nextSequenceNumber, sendTime);
        }
    }

    float ema(float val, float average, float alpha)
    {
        return (alpha * val) + ((1.0f - alpha) * average);
    }

    void GameNetworkService::receive(std::size_t receivedBytes)
    {
        auto receiveTime = getTimestamp();
        spdlog::get("rwe")->debug("Received from endpoint: {0}:{1}", currentRemoteEndpoint.address().to_string(), currentRemoteEndpoint.port());

        auto endpointIt = std::find_if(endpoints.begin(), endpoints.end(), [this](const auto& e) { return currentRemoteEndpoint == e.endpoint; });
        if (endpointIt == endpoints.end())
        {
            // message was from some unknown address, ignore it
            spdlog::get("rwe")->debug("Unknown address, ignoring");
            return;
        }

        proto::NetworkMessage outerMessage;
        outerMessage.ParseFromArray(messageBuffer.data(), receivedBytes);
        if (!outerMessage.has_game_update())
        {
            // message wasn't a game update, ignore it
            spdlog::get("rwe")->debug("Not game update, ignoring");
            return;
        }

        EndpointInfo& endpoint = *endpointIt;

        const auto& message = outerMessage.game_update();

        spdlog::get("rwe")->debug("Received ack to {0} and {1} commands starting at {2}", message.next_command_set_to_receive(), message.command_set_size(), message.next_command_set_to_send());

        SequenceNumber newNextCommandToSend(message.next_command_set_to_receive());
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

        auto extraFrames = static_cast<unsigned int>((endpoint.averageRoundTripTime / 2.0f) / SceneManager::TickInterval);
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

        auto firstRelevantCommandIndex = endpoint.nextCommandToReceive - firstCommandNumber;

        // if the packet is relevant (contains new information), process it
        if (firstRelevantCommandIndex.value < message.command_set_size())
        {
            endpoint.lastReceiveTime = receiveTime;

            for (int i = firstRelevantCommandIndex.value; i < message.command_set_size(); ++i)
            {
                auto commandSet = deserializeCommandSet(message.command_set(i));
                playerCommandService->pushCommands(endpoint.playerId, commandSet);
                endpoint.nextCommandToReceive = SequenceNumber(endpoint.nextCommandToReceive.value + 1);
            }
        }
    }
}
