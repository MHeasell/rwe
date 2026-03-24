#pragma once

#include <array>
#include <atomic>
#include <asio.hpp>
#include <chrono>
#include <functional>
#include <mutex>
#include <network.pb.h>
#include <string>
#include <thread>
#include <vector>

namespace rwe
{
    class LoadingNetworkService
    {
    public:
        enum class Status
        {
            Loading,
            Ready
        };

        struct PlayerInfo
        {
            int playerIndex;
            asio::ip::udp::endpoint endpoint;
            Status status;
            PlayerInfo(int playerIndex, const asio::ip::udp::endpoint& endpoint, Status status) : playerIndex(playerIndex), endpoint(endpoint), status(status) {}
        };

    private:
        std::thread networkThread;

        // state shared between threads
        std::mutex mutex;
        Status loadingStatus{Status::Loading};
        std::vector<PlayerInfo> remoteEndpoints;

        // state owned by the worker thread
        asio::io_context ioContext;
        asio::ip::udp::resolver resolver;
        asio::ip::udp::socket socket;
        std::array<char, 1500> sendBuffer;
        std::array<char, 1500> receiveBuffer;
        asio::ip::udp::endpoint currentRemoteEndpoint;
        asio::steady_timer notifyTimer;

    public:
        LoadingNetworkService();

        virtual ~LoadingNetworkService();

        void addEndpoint(int playerIndex, const std::string& host, const std::string& port);

        asio::ip::udp::endpoint getEndpoint(int playerIndex);

        void setDoneLoading();

        bool areAllClientsReady();

        void waitForAllToBeReady();

        void start(const std::string& port);

    private:
        void run(const std::string& port);

        void onReceive(const asio::error_code& error, std::size_t bytesTransferred);

        void notifyStatus();

        void notifyStatusLoop();

        void startListening();
    };
}
