/*
  ==============================================================================

    BonjourTXTRecord.cpp
    Created: 21 Jun 2020 2:02:22pm
    Author:  Ruurd Adema

  ==============================================================================
*/

#include <dnssd/bonjour/BonjourTXTRecord.h>
#include <map>

using namespace dnssd;

BonjourTXTRecord::BonjourTXTRecord(const std::map<std::string, std::string>& keysValues)
{
    // This way (0 and nullptr) the dns-sd will arrange allocation of a buffer
    TXTRecordCreate(&mTxtRecordRef, 0, nullptr);

    for (auto& keyValue : keysValues)
    {
        setValue(keyValue.first, keyValue.second);
    }
}

BonjourTXTRecord::BonjourTXTRecord(const TxtRecord& txtRecord)
{
    BonjourTXTRecord(txtRecord.txtRecord());
}

BonjourTXTRecord::~BonjourTXTRecord()
{
    TXTRecordDeallocate(&mTxtRecordRef);
}

Error BonjourTXTRecord::setValue(const std::string& key, const std::string& value) noexcept
{
    return Error(TXTRecordSetValue(&mTxtRecordRef, key.c_str(), (uint8_t)value.length(), value.c_str()));
}

Error BonjourTXTRecord::setValue(const std::string& key) noexcept
{
    return Error(TXTRecordSetValue(&mTxtRecordRef, key.c_str(), 0, nullptr));
}

uint16_t BonjourTXTRecord::length() const noexcept
{
    return TXTRecordGetLength(&mTxtRecordRef);
}

const void* BonjourTXTRecord::bytesPtr() const noexcept
{
    return TXTRecordGetBytesPtr(&mTxtRecordRef);
}

std::map<std::string, std::string> BonjourTXTRecord::getTxtRecordFromRawBytes(const unsigned char* txtRecordRawBytes, uint16_t txtRecordLength) noexcept
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
