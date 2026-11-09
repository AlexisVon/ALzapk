// Copyright (c) 2026 AlexisVon

#pragma once

#include "progress.h"

#include <avariant.h>

#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <string>

namespace alx {
    namespace fpacker {
        class decoder;
    }
}

namespace lza {

    enum class open_result { ok,
                             need_password,
                             bad_password,
                             invalid,
                             missing };

    struct extract_progress {
        progress_snapshot stat;
        uint64_t archive_bytes{0};
        uint64_t source_bytes{0};
    };

    struct extract_result {
        enum status { ok,
                      aborted,
                      failed };

        status state{ok};
        std::string failed_file;
        uint8_t error{0};
        uint64_t source_bytes{0};
    };

    /// _total is UINT64_MAX for a per-file failure, with _current holding the error code
    using extract_file_handler = std::function<void(const std::string& _file, uint64_t _total, uint64_t _current)>;
    using extract_progress_handler = std::function<void(const extract_progress&)>;

    /// Size of one index entry: a file's varvec ends with its total size when the count is odd
    uint64_t entry_bytes(const alx::variant& _entry);

    class archive {
    public:
        static std::unique_ptr<archive> open(const std::string& _path, const std::string& _password, open_result& _result);
        ~archive();

        archive(const archive&) = delete;
        archive& operator=(const archive&) = delete;

        const alx::varmap& fsystem() const;
        bool is_encrypt() const;
        uint64_t stream_bytes() const;
        uint64_t total_bytes() const;

        extract_result extract(const std::string& _root, const std::list<std::string>& _path, const std::string& _output,
                               const bool* _abort, const extract_file_handler& _on_file,
                               const extract_progress_handler& _on_progress);

    private:
        explicit archive(alx::fpacker::decoder* _decoder);

    private:
        alx::fpacker::decoder* m_decoder{nullptr};
    };

    const char* extract_error(uint8_t _code);

}
