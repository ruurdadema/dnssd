#pragma once

#include "../common/TxtRecord.h"

namespace dnssd {

    class CommonAdvertiserInterface
    {
    public:
        class Listener
        {
        public:
            virtual ~Listener() = default;
            virtual void onAdvertiserErrorAsync(Error error) const noexcept = 0;
        };

        explicit CommonAdvertiserInterface(const Listener& listener) : mListener(listener) {}

        virtual ~CommonAdvertiserInterface() = default;
        virtual Error registerService(const std::string& serviceName, uint16_t port) noexcept = 0;
        virtual Error registerService(const std::string& serviceName, uint16_t port, const TxtRecord& txtRecord) noexcept = 0;
        virtual void unregisterService() noexcept = 0;

        void callListener(const std::function<void(const Listener&)>& callback) noexcept { callback(mListener); }

    private:
        const Listener& mListener;
    };

} // namespace dnssd
