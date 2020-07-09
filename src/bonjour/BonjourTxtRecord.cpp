/*
  ==============================================================================

    BonjourTxtRecord.cpp
    Created: 21 Jun 2020 2:02:22pm
    Author:  Ruurd Adema

  ==============================================================================
*/

#include <dnssd/bonjour/BonjourTxtRecord.h>
#include <map>

using namespace dnssd;

BonjourTxtRecord::BonjourTxtRecord(const TxtRecord& txtRecord)
{
    // This way (0 and nullptr) the dns-sd will arrange allocation of a buffer
    TXTRecordCreate(&mTxtRecordRef, 0, nullptr);

    for (auto& kv : txtRecord)
    {
        setValue(kv.first, kv.second);
    }
}

BonjourTxtRecord::~BonjourTxtRecord()
{
    TXTRecordDeallocate(&mTxtRecordRef);
}

Error BonjourTxtRecord::setValue(const std::string& key, const std::string& value) noexcept
{
    return Error(TXTRecordSetValue(&mTxtRecordRef, key.c_str(), (uint8_t)value.length(), value.c_str()));
}

Error BonjourTxtRecord::setValue(const std::string& key) noexcept
{
    return Error(TXTRecordSetValue(&mTxtRecordRef, key.c_str(), 0, nullptr));
}

uint16_t BonjourTxtRecord::length() const noexcept
{
    return TXTRecordGetLength(&mTxtRecordRef);
}

const void* BonjourTxtRecord::bytesPtr() const noexcept
{
    return TXTRecordGetBytesPtr(&mTxtRecordRef);
}

std::map<std::string, std::string> BonjourTxtRecord::getTxtRecordFromRawBytes(const unsigned char* txtRecordRawBytes, uint16_t txtRecordLength) noexcept
{
    std::map<std::string, std::string> txtRecord;

    const int   keybuflen = 256;
    char        key[keybuflen];
    uint8_t     valuelen;
    const void* value;

    for (uint16_t i = 0; i < TXTRecordGetCount(txtRecordLength, txtRecordRawBytes); i++)
    {
        TXTRecordGetItemAtIndex(txtRecordLength, txtRecordRawBytes, i, keybuflen, key, &valuelen, &value);
        std::string strKey(key);
        std::string strValue((const char*)value, valuelen);
        txtRecord.insert({strKey, strValue});
    }

    return txtRecord;
}
