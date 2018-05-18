#include "GameNetworkService.h"
#include <rwe/proto/serialization.h>
#include <spdlog/spdlog.h>

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

    void GameNetworkService::submitCommands(const GameNetworkService::CommandSet& commands)
    {
        ioContext.post([this, commands]() {
            for (auto& e : endpoints)
            {
                e.sendBuffer.push_back(commands);
            }
        });
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

    proto::NetworkMessage
    GameNetworkService::createProtoMessage(SequenceNumber nextCommandToSend, SequenceNumber nextCommandToReceive, const std::deque<GameNetworkService::CommandSet>& sendBuffer)
    {
        proto::NetworkMessage outerMessage;
        auto& m = *outerMessage.mutable_game_update();
        m.set_next_command_set_to_send(nextCommandToSend.value);
        m.set_next_command_set_to_receive(nextCommandToReceive.value);

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
        auto message = createProtoMessage(endpoint.nextCommandToSend, endpoint.nextCommandToReceive, endpoint.sendBuffer);
        message.SerializeToArray(messageBuffer.data(), messageBuffer.size());
        socket.send_to(boost::asio::buffer(messageBuffer.data(), message.ByteSize()), endpoint.endpoint);
    }

    void GameNetworkService::receive(std::size_t receivedBytes)
    {
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

        SequenceNumber newNextCommandToSend(message.next_command_set_to_receive());
        while (newNextCommandToSend > endpoint.nextCommandToSend && !endpoint.sendBuffer.empty())
        {
            endpoint.sendBuffer.pop_front();
            endpoint.nextCommandToSend = SequenceNumber(endpoint.nextCommandToSend.value + 1);
        }

        SequenceNumber firstCommandNumber(message.next_command_set_to_send());
        if (firstCommandNumber > endpoint.nextCommandToReceive)
        {
            // message starts with commands too far in the future, ignore it.
            // FIXME: this should probably be an error as it shouldn't ever happen
            spdlog::get("rwe")->error("First command number in message was too high! Expecting no more than {0}, received {1}", endpoint.nextCommandToReceive.value, firstCommandNumber.value);
            return;
        }

        auto firstRelevantCommandIndex = endpoint.nextCommandToReceive - firstCommandNumber;

        for (int i = firstRelevantCommandIndex.value; i < message.command_set_size(); ++i)
        {
            auto commandSet = deserializeCommandSet(message.command_set(i));
            playerCommandService->pushCommands(endpoint.playerId, commandSet);
            endpoint.nextCommandToReceive = SequenceNumber(endpoint.nextCommandToReceive.value + 1);
        }
    }
}
