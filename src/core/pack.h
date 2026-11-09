// Copyright (c) 2026 AlexisVon

#pragma once

#include "progress.h"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace lza {

    struct pack_item {
        std::string path;
        std::string name;
    };

    struct pack_options {
        bool compress{false};
        bool ignore_errors{false};
        std::string password;
    };

    struct pack_progress {
        progress_snapshot stat;
        uint64_t archive_bytes{0};
        uint64_t source_bytes{0};
    };

    struct pack_result {
        enum status { ok,
                      aborted,
                      open_failed,
                      append_failed,
                      finish_failed };

        status state{ok};
        std::string failed_file;
        uint8_t error{0};
        uint64_t source_bytes{0};
    };

    /// _total is UINT64_MAX for a per-file failure, with _current holding the error code
    using pack_file_handler = std::function<void(const std::string& _file, uint64_t _total, uint64_t _current)>;
    using pack_progress_handler = std::function<void(const pack_progress&)>;

    pack_result pack(const std::string& _archive, const std::vector<pack_item>& _items,
                     const pack_options& _options, const bool* _abort,
                     const pack_file_handler& _on_file, const pack_progress_handler& _on_progress);

    const char* pack_error(uint8_t _code);

}
