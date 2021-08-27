#pragma once

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp> // not in asio.hpp in old boost versions
#include <chrono>
#include <deque>
#include <future>
#include <network.pb.h>
#include <random>
#include <rwe/GameHash.h>
#include <rwe/OpaqueId.h>
#include <rwe/OpaqueUnit.h>
#include <rwe/PlayerCommand.h>
#include <rwe/PlayerCommandService.h>
#include <rwe/rwe_time.h>
#include <rwe/sim/GameTime.h>
#include <rwe/sim/PlayerId.h>

namespace rwe
{
    struct SequenceNumberTag;
    using SequenceNumber = OpaqueUnit<int, SequenceNumberTag>;

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

            GameTime nextHashToSend{0};
            GameTime nextHashToReceive{0};

            /**
             * The time at which the last relevant update packet
             * was received from the remote peer.
             * An update packet is relevant
             * if it contains new commands that we haven't seen before.
             */
            std::optional<Timestamp> lastReceiveTime;

            /**
             * The last reported scene time from this peer,
             * adjusted for RTT.
             */
            std::optional<std::pair<SceneTime, Timestamp>> lastKnownSceneTime;

            std::deque<CommandSet> sendBuffer;

            std::deque<GameHash> hashSendBuffer;

            /**
             * Records the time at which we first sent a packet
             * finishing at the given sequence number.
             * This is used for measuring RTT when we receive acks.
             */
            std::deque<std::pair<SequenceNumber, Timestamp>> sendTimes;

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
        std::random_device rd;
        std::default_random_engine gen{rd()};
        std::uniform_int_distribution<int> uniform_dist{};
        PlayerId localPlayerId;
        int port;

        std::thread networkThread;

        boost::asio::io_service ioContext;
        boost::asio::ip::udp::resolver resolver;
        boost::asio::ip::udp::socket socket;
        boost::asio::steady_timer sendTimer;

        std::vector<EndpointInfo> endpoints;

        std::array<char, 1500> sendBuffer;
        std::array<char, 1500> receiveBuffer;
        boost::asio::ip::udp::endpoint currentRemoteEndpoint;

        PlayerCommandService* const playerCommandService;

        SceneTime currentSceneTime{0};

    public:
        GameNetworkService(PlayerId localPlayerId, int port, const std::vector<EndpointInfo>& endpoints, PlayerCommandService* playerCommandService);

        virtual ~GameNetworkService();

        void start();

        /**
         * Submit new information to be sent on the network.
         * @param currentSceneTime The scene time we are currently simulating.
         *                         This is used to inform peers/synchronise simulation speed.
         * @param commands The latest set of player commands.
         *                 These are not related to the current scene time.
         *                 They will be queued up to be sent over the network
         *                 after all the previously submitted commands.
         */
        void submitCommands(SceneTime currentSceneTime, const CommandSet& commands);

        void submitGameHash(GameHash hash);

        SceneTime estimateAvergeSceneTime(SceneTime localSceneTime);

        float getMaxAverageRttMillis();

    private:
        void run();

        void listenForNextMessage();

        void sendLoop();

        void sendToAll();

        void send(EndpointInfo& endpoint);

        void receive(const boost::system::error_code& error, int receivedBytes);
    };
}
