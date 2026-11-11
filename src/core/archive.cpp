// Copyright (c) 2026 AlexisVon

#include "archive.h"

#include <afile.h>
#include <afpacker.h>
#include <astream_ex.h>

namespace {

    const alx::variant* find_child(const alx::varmap& _map, const std::string& _key) {
        for (const auto& it : _map)
            if (it.first == _key) return it.second;
        return nullptr;
    }

    uint64_t selected_bytes(const alx::varmap& _fsys, const std::string& _root, const std::list<std::string>& _path) {
        if (_root.empty() || _root == "*") {
            uint64_t result{0};
            for (const auto& it : _fsys) result += lza::entry_bytes(*it.second);
            return result;
        }

        const alx::variant* node = find_child(_fsys, _root);
        if (node == nullptr) return 0;
        for (const auto& name : _path) {
            if (!node->is<alx::varmap>()) return 0;
            node = find_child(node->to<alx::varmap>(), name);
            if (node == nullptr) return 0;
        }
        return lza::entry_bytes(*node);
    }

}

namespace lza {

    uint64_t entry_bytes(const alx::variant& _entry) {
        if (_entry.is<alx::varmap>()) {
            uint64_t result{0};
            for (const auto& it : _entry.to<alx::varmap>()) result += entry_bytes(*it.second);
            return result;
        }
        if (_entry.is<alx::varvec>()) {
            const alx::varvec& blocks = _entry.to<alx::varvec>();
            return !blocks.empty() && blocks.back().is<alx::uint_64>() ? uint64_t(blocks.back().to<alx::uint_64>()) : 0;
        }
        return 0;
    }

    archive::archive(alx::fpacker::decoder* _decoder, uint8_t _version, bool _encrypt, bool _compress)
        : m_decoder(_decoder), m_version(_version), m_encrypt(_encrypt), m_compress(_compress) {
    }

    archive::~archive() {
        delete m_decoder;
    }

    std::unique_ptr<archive> archive::open(const std::string& _path, const std::string& _password, open_result& _result) {
        const alx::file_info info(_path);
        if (!info.is_exist() || info.is_dir()) {
            _result = open_result::missing;
            return nullptr;
        }

        auto* stream = new alx::istream_file(alx::file_info(_path));
        const alx::fpacker::decoder::pktype type = alx::fpacker::decoder::probe(*stream);
        if (!type.clan) {
            _result = open_result::invalid;
            delete stream;
            return nullptr;
        }
        if (type.encrypt && _password.empty()) {
            _result = open_result::need_password;
            delete stream;
            return nullptr;
        }

        uint8_t code{0};
        alx::fpacker::decoder* decoder = alx::fpacker::decoder::create(stream, _password, &code);
        if (decoder == nullptr) {
            switch (code) {
            case alx::fpacker::decoder::badversion:
            case alx::fpacker::decoder::badparams: _result = open_result::invalid; break;
            case alx::fpacker::decoder::badauth: _result = open_result::bad_password; break;
            default: _result = type.encrypt ? open_result::bad_password : open_result::invalid; break;
            }
            return nullptr;
        }

        _result = open_result::ok;
        return std::unique_ptr<archive>(new archive(decoder, type.version, type.encrypt, type.cmps));
    }

    const alx::varmap& archive::fsystem() const {
        return m_decoder->fsystem();
    }

    uint8_t archive::version() const {
        return m_version;
    }

    bool archive::is_encrypt() const {
        return m_encrypt;
    }

    bool archive::is_compress() const {
        return m_compress;
    }

    const alx::varmap& archive::info() const {
        return m_decoder->archive();
    }

    uint64_t archive::stream_bytes() const {
        return uint64_t(m_decoder->get_stream()->total());
    }

    uint64_t archive::total_bytes() const {
        uint64_t result{0};
        for (const auto& it : m_decoder->fsystem()) result += entry_bytes(*it.second);
        return result;
    }

    extract_result archive::extract(const std::string& _root, const std::list<std::string>& _path, const std::string& _output,
                                    const bool* _abort, const extract_file_handler& _on_file,
                                    const extract_progress_handler& _on_progress) {
        extract_result result;
        result.source_bytes = selected_bytes(m_decoder->fsystem(), _root, _path);

        m_decoder->set_abort_flag(_abort);

        uint64_t done{0};
        progress_reporter reporter(result.source_bytes);
        m_decoder->set_print_rtmsg([&](const std::string& _name, uint64_t _total, uint64_t _current) {
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
                extract_progress info;
                info.stat = snapshot;
                info.archive_bytes = stream_bytes();
                info.source_bytes = result.source_bytes;
                _on_progress(info);
            }
            if (_on_file) _on_file(_name, _total, _current);
        });

        const uint8_t ret = m_decoder->save(_root, _path, alx::file_info(_output));
        if (ret == alx::fpacker::decoder::success) result.state = extract_result::ok;
        else if (_abort && *_abort) result.state = extract_result::aborted;
        else {
            result.state = extract_result::failed;
            result.error = ret;
        }
        return result;
    }

    const char* extract_error(uint8_t _code) {
        return alx::fpacker::decoder::error_message(_code);
    }

}
