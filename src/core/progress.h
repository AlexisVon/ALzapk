// Copyright (c) 2026 AlexisVon

#pragma once

#include <chrono>
#include <cstdint>

namespace lza {

    struct progress_snapshot {
        int percent{0};
        double speed_bps{0.0};
        uint64_t eta_ms{0};
    };

    class progress_reporter {
    public:
        explicit progress_reporter(uint64_t _total_bytes);

        bool tick(uint64_t _done_bytes, progress_snapshot& _out);

    private:
        using clock = std::chrono::steady_clock;

        uint64_t m_total{0};
        uint64_t m_last{0};
        clock::time_point m_stamp;
    };

}
