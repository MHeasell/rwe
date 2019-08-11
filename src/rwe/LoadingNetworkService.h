#pragma once

#include <array>
#include <atomic>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp> // not in asio.hpp in old boost versions
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

    private:
        std::thread networkThread;

        // state shared between threads
        std::mutex mutex;
        Status loadingStatus{Status::Loading};
        std::vector<std::pair<boost::asio::ip::udp::endpoint, Status>> remoteEndpoints;

        // state owned by the worker thread
        boost::asio::io_service ioContext;
        boost::asio::ip::udp::resolver resolver;
        boost::asio::ip::udp::socket socket;
        std::array<char, 1500> messageBuffer;
        boost::asio::ip::udp::endpoint currentRemoteEndpoint;
        boost::asio::steady_timer notifyTimer;

    public:
        LoadingNetworkService();

        virtual ~LoadingNetworkService();

        void addEndpoint(const std::string& host, const std::string& port);

        void setDoneLoading();

        bool areAllClientsReady();

        void waitForAllToBeReady();

        void start(const std::string& host, const std::string& port);

    private:
        void run(const std::string& host, const std::string& port);

        void onReceive(const boost::system::error_code& error, std::size_t bytesTransferred);

        void notifyStatus();

        void notifyStatusLoop();

        void startListening();
    };
}
