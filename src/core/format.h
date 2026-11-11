// Copyright (c) 2026 AlexisVon

#pragma once

#include <avariant.h>

#include <cstdint>
#include <string>

namespace lza {

    std::string size_string(uint64_t _size);
    std::string time_string(uint64_t _ms);
    std::string variant_string(const alx::variant& _value);

}
