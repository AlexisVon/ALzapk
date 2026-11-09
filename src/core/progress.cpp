// Copyright (c) 2026 AlexisVon

#include "progress.h"

namespace lza {

    progress_reporter::progress_reporter(uint64_t _total_bytes)
        : m_total(_total_bytes), m_stamp(clock::now()) {
    }

    bool progress_reporter::tick(uint64_t _done_bytes, progress_snapshot& _out) {
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - m_stamp).count();
        if (elapsed < 1000) return false;

        const double speed = double(_done_bytes - m_last) * 1000.0 / double(elapsed);
        _out.speed_bps = speed;
        _out.percent = m_total ? int(_done_bytes * 100 / m_total) : 0;
        _out.eta_ms = speed > 0.0 && _done_bytes < m_total ? uint64_t(double(m_total - _done_bytes) * 1000.0 / speed) : 0;
        m_last = _done_bytes;
        m_stamp = clock::now();
        return true;
    }

}
