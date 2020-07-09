#pragma once

#include <string>
#include <map>

#include "dnssd/common/Error.h"
#include "dnssd/common/TxtRecord.h"

namespace dnssd {

    class BonjourTxtRecord
    {
    public:
        explicit BonjourTxtRecord(const TxtRecord& txtRecord);
        ~BonjourTxtRecord();

        Error setValue(const std::string& key, const std::string& value) noexcept;
        Error setValue(const std::string& key) noexcept;

        uint16_t length() const noexcept;
        const void* bytesPtr() const noexcept;

        static TxtRecord getTxtRecordFromRawBytes(const unsigned char* txtRecord, uint16_t txtRecordLength) noexcept;

    private:
        TXTRecordRef mTxtRecordRef;
    };

} // namespace dnssd
