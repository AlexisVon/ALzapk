// Copyright (c) 2026 AlexisVon

#include "pack.h"

#include <afile.h>
#include <afpacker.h>
#include <astream_ex.h>

namespace {

    uint64_t total_source_bytes(const std::vector<lza::pack_item>& _items) {
        uint64_t result{0};
        for (const auto& item : _items) {
            const alx::file_info info(item.path);
            result += info.is_dir() ? alx::file_info::calsize(info, true) : info.size();
        }
        return result;
    }

}

namespace lza {

    pack_result pack(const std::string& _archive, const std::vector<pack_item>& _items,
                     const pack_options& _options, const bool* _abort,
                     const pack_file_handler& _on_file, const pack_progress_handler& _on_progress) {
        pack_result result;
        result.source_bytes = total_source_bytes(_items);

        alx::fpacker::encoder packer(_options.compress, _options.password);
        packer.set_abort_flag(_abort);
        packer.set_igerr_linkfile(true);
        packer.set_igerr_notexist(_options.ignore_errors);

        uint64_t done{0};
        progress_reporter reporter(result.source_bytes);
        packer.set_print_rtmsg([&](const std::string& _name, uint64_t _total, uint64_t _current) {
            if (_total == uint64_t(-1)) {
                result.failed_file = _name;
                result.error = uint8_t(_current);
                if (_on_file) _on_file(_name, _total, _current);
                return;
            }

            const uint64_t execed = done + _current;
            if (_total == _current) done += _current;

            progress_snapshot snapshot;
            if (reporter.tick(execed, snapshot) && _on_progress) {
                pack_progress info;
                info.stat = snapshot;
                info.archive_bytes = packer.get_stream() ? packer.get_stream()->total() : 0;
                info.source_bytes = result.source_bytes;
                _on_progress(info);
            }
            if (_on_file) _on_file(_name, _total, _current);
        });

        if (!packer.start(new alx::ostream_file(alx::file_info(_archive), false))) {
            result.state = pack_result::open_failed;
            return result;
        }

        uint8_t ret{packer.success};
        for (const auto& item : _items) {
            ret = packer.append(alx::file_info(item.path), item.name);
            if (ret != packer.success) break;
        }

        if (ret != packer.success) {
            result.error = ret;
            result.state = _abort && *_abort ? pack_result::aborted : pack_result::append_failed;
            return result;
        }
        if (!packer.finish()) {
            result.state = pack_result::finish_failed;
            return result;
        }

        result.state = pack_result::ok;
        return result;
    }

    const char* pack_error(uint8_t _code) {
        return alx::fpacker::encoder::error_message(_code);
    }

}
