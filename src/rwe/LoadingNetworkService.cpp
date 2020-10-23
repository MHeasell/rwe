#include "LoadingNetworkService.h"
#include <rwe/Index.h>
#include <rwe/network_util.h>
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

    void LoadingNetworkService::addEndpoint(int playerIndex, const std::string& host, const std::string& port)
    {
        std::scoped_lock<std::mutex> lock(mutex);

        // boost guarantees that resolve returns non-empty
        remoteEndpoints.emplace_back(playerIndex, *resolver.resolve(boost::asio::ip::udp::resolver::query(host, port)), Status::Loading);
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
            if (p.status != Status::Ready)
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

    void LoadingNetworkService::start(const std::string& port)
    {
        networkThread = std::thread(&LoadingNetworkService::run, this, port);
    }

    void LoadingNetworkService::run(const std::string& port)
    {
        try
        {
            spdlog::get("rwe")->debug("Opening listen socket on port {}", port);
            auto endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v6(), std::stoi(port));
            socket.open(endpoint.protocol());
            socket.bind(endpoint);

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
            return;
        }

        std::scoped_lock<std::mutex> lock(mutex);
        spdlog::get("rwe")->debug("Received network message from {0} {1}, size {2}", currentRemoteEndpoint.address().to_string(), currentRemoteEndpoint.port(), bytesTransferred);
        auto it = std::find_if(remoteEndpoints.begin(), remoteEndpoints.end(), [this](const auto& p) { return p.endpoint == currentRemoteEndpoint; });
        if (it == remoteEndpoints.end())
        {
            // message from some unknown address, ignore
            spdlog::get("rwe")->debug("Sender was unknown, aborting");
            return;
        }

        spdlog::get("rwe")->debug("Sender was recognised");

        if (bytesTransferred < 4)
        {
            spdlog::get("rwe")->error("Received message is too short, ignoring");
            return;
        }

        auto receivedCrc = readInt(&receiveBuffer[bytesTransferred - 4]);
        auto computedCrc = computeCrc(receiveBuffer.data(), bytesTransferred - 4);
        if (receivedCrc != computedCrc)
        {
            spdlog::get("rwe")->error("Message CRC incorrect, ignoring");
            return;
        }

        proto::NetworkMessage message;
        message.ParseFromArray(receiveBuffer.data(), bytesTransferred - 4);

        if (!message.has_loading_status())
        {
            spdlog::get("rwe")->debug("Sender is already in game!");
            it->status = Status::Ready;
            return;
        }

        switch (message.loading_status().status())
        {
            case proto::LoadingStatusMessage_Status_Loading:
                spdlog::get("rwe")->debug("Sender is loading");
                it->status = Status::Loading;
                break;
            case proto::LoadingStatusMessage_Status_Ready:
                spdlog::get("rwe")->debug("Sender is ready");
                it->status = Status::Ready;
                break;
            default:
                throw std::logic_error("Unhandled loading status");
        }
    }

    void LoadingNetworkService::notifyStatus()
    {
        std::scoped_lock<std::mutex> lock(mutex);
        spdlog::get("rwe")->debug("Notifying peers about loading status");

        proto::NetworkMessage outerMessage;

        {
            auto& innerMessage = *outerMessage.mutable_loading_status();
            switch (loadingStatus)
            {
                case Status::Loading:
                    spdlog::get("rwe")->debug("we are loading");
                    innerMessage.set_status(proto::LoadingStatusMessage_Status_Loading);
                    break;
                case Status::Ready:
                    spdlog::get("rwe")->debug("we are ready");
                    innerMessage.set_status(proto::LoadingStatusMessage_Status_Ready);
                    break;
                default:
                    throw std::logic_error("Unhandled loading status");
            }
        }

        auto messageSize = outerMessage.ByteSize();
        if (messageSize > getSize(sendBuffer) - 4)
        {
            throw std::runtime_error("Message to be sent was bigger than buffer size");
        }
        if (!outerMessage.SerializeToArray(sendBuffer.data(), sendBuffer.size()))
        {
            throw std::runtime_error("Failed to serialize message to buffer");
        }

        // throw in a CRC to verify the message
        writeInt(&sendBuffer[messageSize], computeCrc(sendBuffer.data(), messageSize));

        for (const auto& p : remoteEndpoints)
        {
            spdlog::get("rwe")->debug("Sending notification to {0} {1}, size {2}", p.endpoint.address().to_string(), p.endpoint.port(), outerMessage.ByteSize());
            socket.send_to(boost::asio::buffer(sendBuffer.data(), messageSize + 4), p.endpoint);
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
            boost::asio::buffer(receiveBuffer.data(), receiveBuffer.size()),
            currentRemoteEndpoint,
            [this](const auto& error, const auto& bytesTransferred) {
                onReceive(error, bytesTransferred);
                startListening();
            });
    }
    boost::asio::ip::udp::endpoint LoadingNetworkService::getEndpoint(int playerIndex)
    {
        auto it = std::find_if(remoteEndpoints.begin(), remoteEndpoints.end(), [&](const auto& e) { return e.playerIndex == playerIndex; });
        if (it == remoteEndpoints.end())
        {
            throw std::runtime_error("Endpoint not found for index " + std::to_string(playerIndex));
        }
        return it->endpoint;
    }
}
