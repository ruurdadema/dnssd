/*
  ==============================================================================

    BonjourTXTRecord.h
    Created: 21 Jun 2020 2:02:22pm
    Author:  Ruurd Adema

  ==============================================================================
*/

#pragma once

#include <string>
#include <map>

#include "dnssd/common/Error.h"
#include "dnssd/common/TxtRecord.h"

namespace dnssd {

    class BonjourTXTRecord
    {
    public:
        explicit BonjourTXTRecord(const std::map<std::string, std::string>& keysValues);
        explicit BonjourTXTRecord(const TxtRecord& txtRecord);
        ~BonjourTXTRecord();

        Error setValue(const std::string& key, const std::string& value) noexcept;
        Error setValue(const std::string& key) noexcept;

        uint16_t length() const noexcept;
        const void* bytesPtr() const noexcept;

        static std::map<std::string, std::string> getTxtRecordFromRawBytes(const unsigned char* txtRecord, uint16_t txtRecordLength) noexcept;

    private:
        TXTRecordRef mTxtRecordRef;
    };

} // namespace dnssd
