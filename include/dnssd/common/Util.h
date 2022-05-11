//
// Created by Ruurd Adema on 12/08/2021.
// Copyright (c) 2021 Owllab. All rights reserved.
//

#pragma once

namespace dnssd
{

/**
 * Small utility class for ignoring compiler warnings about variables not being used.
 */
template <typename... Types> void ignore (Types&&...) noexcept {}

} // namespace dnssd
