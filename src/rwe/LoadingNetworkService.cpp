#include "LoadingNetworkService.h"
#include <spdlog/spdlog.h>

namespace rwe
{
    LoadingNetworkService::LoadingNetworkService()
        : resolver(ioContext), socket(ioContext), notifyTimer(ioContext)
    {
    }

    LoadingNetworkService::~LoadingNetworkService()
    {
        if (networkThread.joinable())
        {
            ioContext.stop();
            networkThread.join();
        }
    }

    void LoadingNetworkService::addEndpoint(const std::string& host, const std::string& port)
    {
        std::scoped_lock<std::mutex> lock(mutex);

        // boost guarantees that resolve returns non-empty
        remoteEndpoints.emplace_back(*resolver.resolve(boost::asio::ip::udp::resolver::query(host, port)), Status::Loading);
    }

    void LoadingNetworkService::setDoneLoading()
    {
        std::scoped_lock<std::mutex> lock(mutex);
        loadingStatus = Status::Ready;
    }

    bool LoadingNetworkService::areAllClientsReady()
    {
        std::scoped_lock<std::mutex> lock(mutex);
        for (const auto& p : remoteEndpoints)
        {
            if (p.second != Status::Ready)
            {
                return false;
            }
        }
        return true;
    }

    void LoadingNetworkService::waitForAllToBeReady()
    {
        // crude, but it works
        do
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        } while (!areAllClientsReady());
    }

    void LoadingNetworkService::start(const std::string& host, const std::string& port)
    {
        networkThread = std::thread(&LoadingNetworkService::run, this, host, port);
    }

    void LoadingNetworkService::run(const std::string& host, const std::string& port)
    {
        try
        {
            auto localEndpoint = *resolver.resolve(boost::asio::ip::udp::resolver::query(host, port));
            spdlog::get("rwe")->debug("Binding to {0} {1}", localEndpoint.endpoint().address().to_string(), localEndpoint.endpoint().port());
            socket.open(localEndpoint.endpoint().protocol());
            socket.bind(localEndpoint);

            startListening();

            notifyStatusLoop();

            ioContext.run();
        }
        catch (const std::exception& e)
        {
            spdlog::get("rwe")->error("Network thread died with error: {0}", e.what());
        }
    }

    void LoadingNetworkService::onReceive(const boost::system::error_code& error, std::size_t bytesTransferred)
    {
        if (error)
        {
            spdlog::get("rwe")->error("Received network error from {0} port {1}: {2}", currentRemoteEndpoint.address().to_string(), currentRemoteEndpoint.port(), error.message());
            startListening();
            return;
        }

        std::scoped_lock<std::mutex> lock(mutex);
        spdlog::get("rwe")->debug("Received network message from {0} {1}, size {2}", currentRemoteEndpoint.address().to_string(), currentRemoteEndpoint.port(), bytesTransferred);
        auto it = std::find_if(remoteEndpoints.begin(), remoteEndpoints.end(), [this](const auto& p) { return p.first == currentRemoteEndpoint; });
        if (it == remoteEndpoints.end())
        {
            // message from some unknown address, ignore
            spdlog::get("rwe")->debug("Sender was unknown, aborting");
            return;
        }

        spdlog::get("rwe")->debug("Sender was recognised");

        LoadingStatusMessage message;
        message.ParseFromArray(messageBuffer.data(), bytesTransferred);

        switch (message.status())
        {
            case LoadingStatusMessage_Status_Loading:
                spdlog::get("rwe")->debug("Sender is loading");
                it->second = Status::Loading;
                break;
            case LoadingStatusMessage_Status_Ready:
                spdlog::get("rwe")->debug("Sender is ready");
                it->second = Status::Ready;
                break;
            default:
                throw std::logic_error("Unhandled loading status");
        }

        startListening();
    }

    void LoadingNetworkService::notifyStatus()
    {
        std::scoped_lock<std::mutex> lock(mutex);
        spdlog::get("rwe")->debug("Notifying peers about loading status");

        LoadingStatusMessage message;
        switch (loadingStatus)
        {
            case Status::Loading:
                spdlog::get("rwe")->debug("we are loading");
                message.set_status(LoadingStatusMessage_Status_Loading);
                break;
            case Status::Ready:
                spdlog::get("rwe")->debug("we are ready");
                message.set_status(LoadingStatusMessage_Status_Ready);
                break;
            default:
                throw std::logic_error("Unhandled loading status");
        }

        message.SerializeToArray(messageBuffer.data(), messageBuffer.size());

        for (const auto& p : remoteEndpoints)
        {
            spdlog::get("rwe")->debug("Sending notification to {0} {1}, size {2}", p.first.address().to_string(), p.first.port(), message.ByteSize());
            socket.send_to(boost::asio::buffer(messageBuffer.data(), message.ByteSize()), p.first);
        }
    }

    void LoadingNetworkService::notifyStatusLoop()
    {
        notifyStatus();
        notifyTimer.expires_from_now(std::chrono::milliseconds(100));
        notifyTimer.async_wait([this](const boost::system::error_code& error) {
            if (error)
            {
                spdlog::get("rwe")->error("Received error from network notify timer: {0}", error.message());
                return;
            }
            notifyStatusLoop();
        });
    }

    void LoadingNetworkService::startListening()
    {
        // go back to waiting for the next message
        socket.async_receive_from(
            boost::asio::buffer(messageBuffer.data(), messageBuffer.size()),
            currentRemoteEndpoint,
            std::bind(&LoadingNetworkService::onReceive, this, std::placeholders::_1, std::placeholders::_2));
    }
}
