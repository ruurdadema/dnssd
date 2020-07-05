//
// Created by Ruurd Adema on 05/07/2020.
//

#pragma once

#include <map>
#include <string>
#include <utility>

namespace dnssd {

    class TxtRecord
    {
    public:
        TxtRecord() = default;
        explicit TxtRecord(std::map<std::string, std::string> keysValues) : mTxtRecord(std::move(keysValues)) {}

        const std::map<std::string, std::string>& txtRecord() const noexcept { return mTxtRecord; }
    private:
        std::map<std::string, std::string> mTxtRecord;
    };

}
