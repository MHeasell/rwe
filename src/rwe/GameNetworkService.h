#ifndef RWE_GAMENETWORKSERVICE_H
#define RWE_GAMENETWORKSERVICE_H

#include <boost/asio.hpp>
#include <deque>
#include <network.pb.h>
#include <rwe/OpaqueId.h>
#include <rwe/OpaqueUnit.h>
#include <rwe/PlayerCommand.h>
#include <rwe/PlayerCommandService.h>
#include <rwe/PlayerId.h>
#include <thread>
#include <chrono>

namespace rwe
{
    struct SequenceNumberTag;
    using SequenceNumber = OpaqueUnit<unsigned int, SequenceNumberTag>;

    class GameNetworkService
    {
    public:
        using CommandSet = std::vector<PlayerCommand>;
        struct EndpointInfo
        {
            PlayerId playerId;
            boost::asio::ip::udp::endpoint endpoint;

            SequenceNumber nextCommandToSend{0};
            SequenceNumber nextCommandToReceive{0};

            /**
             * The time at which the last update packet
             * was received from the remote peer.
             */
            std::optional<std::chrono::time_point<std::chrono::steady_clock>> lastReceiveTime;
            std::deque<CommandSet> sendBuffer;

            /**
             * Records the time at which we first sent a packet
             * finishing at the given sequence number.
             * This is used for measuring RTT when we receive acks.
             */
            std::deque<std::pair<SequenceNumber, std::chrono::time_point<std::chrono::steady_clock>>> sendTimes;

            /**
             * Exponential moving average of round trip time
             * for communication between us and the remote peer.
             */
            float averageRoundTripTime{0};

            EndpointInfo(const PlayerId& playerId, const boost::asio::ip::udp::endpoint& endpoint)
                : playerId(playerId), endpoint(endpoint)
            {
            }
        };

    private:
        boost::asio::ip::udp::endpoint localEndpoint;

        std::thread networkThread;

        boost::asio::io_service ioContext;
        boost::asio::ip::udp::resolver resolver;
        boost::asio::ip::udp::socket socket;
        boost::asio::steady_timer sendTimer;

        std::vector<EndpointInfo> endpoints;

        std::array<char, 1500> messageBuffer;
        boost::asio::ip::udp::endpoint currentRemoteEndpoint;

        PlayerCommandService* const playerCommandService;

    public:
        GameNetworkService(const boost::asio::ip::udp::endpoint& localEndpoint, const std::vector<EndpointInfo>& endpoints, PlayerCommandService* playerCommandService);

        virtual ~GameNetworkService();

        void start();

        void submitCommands(const CommandSet& commands);

    private:
        void run();

        void listenForNextMessage();

        void onReceive(const boost::system::error_code& error, std::size_t bytesTransferred);

        proto::NetworkMessage createProtoMessage(SequenceNumber nextCommandToSend, SequenceNumber nextCommandToReceive, const std::deque<CommandSet>& sendBuffer);

        void sendLoop();

        void sendToAll();

        void send(EndpointInfo& endpoint);

        void receive(std::size_t receivedBytes);
    };
}

#endif
