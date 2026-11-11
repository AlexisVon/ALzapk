// Copyright (c) 2026 AlexisVon

#include "format.h"

#include <cstdio>

namespace lza {

    std::string size_string(uint64_t _size) {
        static const char* units[]{"B", "K", "M", "G", "T", "P"};
        int uidx{0};
        double sidx{double(_size)};
        while (sidx >= 1024.0 && uidx < 5) {
            sidx /= 1024.0;
            uidx++;
        }

        char buf[32];
        if (uidx == 0) snprintf(buf, sizeof(buf), "%llu B", (unsigned long long) _size);
        else snprintf(buf, sizeof(buf), "%.2f %s", sidx, units[uidx]);
        return buf;
    }

    std::string time_string(uint64_t _ms) {
        static constexpr uint64_t minute = 60;
        static constexpr uint64_t hour = 60 * 60;
        static constexpr uint64_t day = 60 * 60 * 24;

        char buf[40];
        if (_ms < 1000) {
            snprintf(buf, sizeof(buf), "%llums", (unsigned long long) _ms);
            return buf;
        }
        const uint64_t seconds = _ms / 1000;
        if (seconds < minute) snprintf(buf, sizeof(buf), "%llus", (unsigned long long) seconds);
        else if (seconds < hour) snprintf(buf, sizeof(buf), "%llum%llus", (unsigned long long) (seconds / minute), (unsigned long long) (seconds % minute));
        else if (seconds < day)
            snprintf(buf, sizeof(buf), "%lluh%llum%llus", (unsigned long long) (seconds / hour), (unsigned long long) (seconds % hour / minute),
                     (unsigned long long) (seconds % minute));
        else
            snprintf(buf, sizeof(buf), "%llud%lluh%llum%llus", (unsigned long long) (seconds / day), (unsigned long long) (seconds % day / hour),
                     (unsigned long long) (seconds % hour / minute), (unsigned long long) (seconds % minute));
        return buf;
    }

    std::string variant_string(const alx::variant& _value) {
        if (_value.null()) return "null";
        if (_value.is<std::string>()) return _value.to<std::string>();
        if (_value.is<bool>()) return _value.to<bool>() ? "true" : "false";
        if (_value.is_integer()) return std::to_string(_value.to<long long>());
        if (_value.is_uinteger()) return std::to_string(_value.to<unsigned long long>());
        if (_value.is<float>() || _value.is<double>()) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%g", _value.to<double>());
            return buf;
        }
        if (_value.is<alx::bytes>()) return "<bytes>";
        if (_value.is_map()) return "<map>";
        if (_value.is_vec()) return "<vector>";
        if (_value.is_lst()) return "<list>";
        return "<value>";
    }

}
