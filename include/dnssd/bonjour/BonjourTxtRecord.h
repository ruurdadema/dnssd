#pragma once

#include <map>
#include <string>

#include "dnssd/common/Result.h"
#include "dnssd/common/TxtRecord.h"

namespace dnssd
{

/**
 * Class for holding and working with a TXTRecordRef
 */
class BonjourTxtRecord
{
public:
    explicit BonjourTxtRecord (const TxtRecord& txtRecord);
    ~BonjourTxtRecord();

    /**
     * Sets a value inside the TXT record.
     * @param key Key.
     * @param value Value.
     * @return An Result indicating success or failure.
     */
    Result setValue (const std::string& key, const std::string& value) noexcept;

    /**
     * Sets an empty value for key inside the TXT record.
     * @param key Key.
     * @return An Result indicating success or failure.
     */
    Result setValue (const std::string& key) noexcept;

    /**
     * @return Returns the length of the TXT record.
     */
    uint16_t length() const noexcept;

    /**
     * @return Returns a pointer to the TXT record data. This pointer will be valid for as long as this instance lives.
     */
    const void* bytesPtr() const noexcept;

    /**
     * Creates a TxtRecord from raw TXT record bytes.
     * @param txtRecord The txt record data.
     * @param txtRecordLength The length of the txt record.
     * @return The filled TxtRecord.
     */
    static TxtRecord getTxtRecordFromRawBytes (const unsigned char* txtRecord, uint16_t txtRecordLength) noexcept;

private:
    TXTRecordRef mTxtRecordRef;
};

} // namespace dnssd
