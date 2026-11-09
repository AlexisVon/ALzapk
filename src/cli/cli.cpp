// Copyright (c) 2026 AlexisVon

#include "cli.h"

#include <core/archive.h>
#include <core/format.h>
#include <core/pack.h>

#include <afile.h>

#include <cstdio>
#include <list>
#include <map>
#include <string>
#include <vector>

#ifndef LZA_VERSION
#    define LZA_VERSION "1.0.0"
#endif

namespace {

    const char* const k_help =
        "Usage: lza-cli <command> [switches] <archive.lza> [<item>...]\n"
        "\n"
        "Commands:\n"
        "  a   Add files or directories to a new archive\n"
        "  x   Extract entries (the whole archive when no item is given)\n"
        "  l   List the archive contents\n"
        "\n"
        "Switches:\n"
        "  -c           Compress file data (LZ4)\n"
        "  -p<password> Encrypt with a password\n"
        "  -o<dir>      Output directory for x (default: current directory)\n"
        "  -i           Ignore missing sources when packing\n"
        "  -h, --help   Show this help\n"
        "  -v, --version Show version\n"
        "\n"
        "Item paths use '/' as the separator, e.g. root/dir/file.\n";

    struct options {
        char command{0};
        std::string archive;
        std::vector<std::string> items;
        bool compress{false};
        bool ignore_errors{false};
        bool output_given{false};
        std::string password;
        std::string output{"."};
    };

    int parse(int _argc, char** _argv, options& _opt) {
        if (_argc < 2) {
            fputs(k_help, stderr);
            return 2;
        }

        const std::string command = _argv[1];
        if (command == "-h" || command == "--help") {
            fputs(k_help, stdout);
            return 1;
        }
        if (command == "-v" || command == "--version") {
            fputs(LZA_VERSION "\n", stdout);
            return 1;
        }
        if (command.size() != 1 || (command[0] != 'a' && command[0] != 'l' && command[0] != 'x')) {
            fprintf(stderr, "lza-cli: unknown command \"%s\"\n\n", command.c_str());
            fputs(k_help, stderr);
            return 2;
        }
        _opt.command = command[0];

        std::vector<std::string> positional;
        for (int i = 2; i < _argc; i++) {
            const std::string arg = _argv[i];
            if (arg == "-h" || arg == "--help") {
                fputs(k_help, stdout);
                return 1;
            }
            if (arg.size() > 1 && arg[0] == '-') {
                switch (arg[1]) {
                case 'c': _opt.compress = true; break;
                case 'i': _opt.ignore_errors = true; break;
                case 'p':
                    if (arg.size() == 2) {
                        fputs("lza-cli: -p requires an attached password\n", stderr);
                        return 2;
                    }
                    _opt.password = arg.substr(2);
                    break;
                case 'o':
                    if (arg.size() == 2) {
                        fputs("lza-cli: -o requires an attached directory\n", stderr);
                        return 2;
                    }
                    _opt.output = arg.substr(2);
                    _opt.output_given = true;
                    break;
                default:
                    fprintf(stderr, "lza-cli: unknown switch \"%s\"\n", arg.c_str());
                    return 2;
                }
            } else positional.push_back(arg);
        }

        if (positional.empty()) {
            fputs("lza-cli: missing archive name\n", stderr);
            return 2;
        }
        _opt.archive = positional.front();
        _opt.items.assign(positional.begin() + 1, positional.end());

        if (_opt.command == 'a') {
            if (_opt.items.empty()) {
                fputs("lza-cli: a: no input files\n", stderr);
                return 2;
            }
            if (_opt.output_given) {
                fputs("lza-cli: -o is only valid for x\n", stderr);
                return 2;
            }
        } else if (_opt.command == 'l') {
            if (!_opt.items.empty()) {
                fputs("lza-cli: l: unexpected item arguments\n", stderr);
                return 2;
            }
            if (_opt.compress || _opt.ignore_errors || _opt.output_given) {
                fputs("lza-cli: -c, -i and -o are only valid for a or x\n", stderr);
                return 2;
            }
        } else {
            if (_opt.compress || _opt.ignore_errors) {
                fputs("lza-cli: -c and -i are only valid for a\n", stderr);
                return 2;
            }
        }
        return 0;
    }

    int report_open_error(const std::string& _archive, lza::open_result _result) {
        switch (_result) {
        case lza::open_result::need_password: fprintf(stderr, "lza-cli: %s: encrypted archive, use -p<password>\n", _archive.c_str()); return 1;
        case lza::open_result::bad_password: fprintf(stderr, "lza-cli: %s: wrong password\n", _archive.c_str()); return 1;
        case lza::open_result::invalid: fprintf(stderr, "lza-cli: %s: not a valid archive\n", _archive.c_str()); return 1;
        case lza::open_result::missing: fprintf(stderr, "lza-cli: %s: no such file or directory\n", _archive.c_str()); return 1;
        case lza::open_result::ok: return 0;
        }
        return 1;
    }

    int report_extract_error(const lza::extract_result& _result, const std::string& _target) {
        if (_result.state == lza::extract_result::aborted) {
            fputs("lza-cli: aborted\n", stderr);
            return 1;
        }
        const std::string where = _result.failed_file.empty() ? _target : _result.failed_file;
        fprintf(stderr, "lza-cli: %s: %s\n", where.c_str(), lza::extract_error(_result.error));
        return 1;
    }

    int command_add(const options& _opt) {
        std::vector<lza::pack_item> items;
        std::map<std::string, std::string> names;

        for (const auto& source : _opt.items) {
            const alx::file_info info(source);
            if (!info.is_exist() && !_opt.ignore_errors) {
                fprintf(stderr, "lza-cli: %s: no such file or directory\n", source.c_str());
                return 1;
            }
            if (info.is_link()) {
                fprintf(stderr, "lza-cli: %s: symbolic links are not supported\n", source.c_str());
                return 1;
            }

            const std::string name = info.name();
            if (name.empty() || name == "." || name == "..") {
                fprintf(stderr, "lza-cli: %s: invalid entry name\n", source.c_str());
                return 1;
            }

            const auto inserted = names.emplace(name, source);
            if (!inserted.second) {
                fprintf(stderr, "lza-cli: duplicate entry name \"%s\": %s and %s\n", name.c_str(), inserted.first->second.c_str(), source.c_str());
                return 1;
            }
            items.push_back({info.path(), name});
        }

        lza::pack_options pack_options;
        pack_options.compress = _opt.compress;
        pack_options.ignore_errors = _opt.ignore_errors;
        pack_options.password = _opt.password;

        printf("Creating archive: %s\n", _opt.archive.c_str());
        bool warned{false};
        const auto on_file = [&warned](const std::string& _file, uint64_t _total, uint64_t _current) {
            if (_total == uint64_t(-1)) {
                warned = true;
                fprintf(stderr, "lza-cli: %s: %s (skipped)\n", _file.c_str(), lza::pack_error(uint8_t(_current)));
                return;
            }
            if (_total == _current) printf("+ %s\n", _file.c_str());
        };

        const lza::pack_result result = lza::pack(_opt.archive, items, pack_options, nullptr, on_file, nullptr);
        switch (result.state) {
        case lza::pack_result::ok: {
            const alx::file_info archive_info(_opt.archive);
            const uint64_t archive_bytes = archive_info.size();
            if (archive_info.is_exist()) {
                const double ratio = result.source_bytes ? double(archive_bytes) * 100.0 / double(result.source_bytes) : 0.0;
                printf("Everything is Ok\n  %s -> %s (%.1f%%)\n", lza::size_string(result.source_bytes).c_str(),
                       lza::size_string(archive_bytes).c_str(), ratio);
            } else printf("Everything is Ok\n");
            return 0;
        }
        case lza::pack_result::aborted: fputs("lza-cli: aborted\n", stderr); return 1;
        case lza::pack_result::open_failed:
            fprintf(stderr, "lza-cli: %s: cannot write the archive\n", _opt.archive.c_str());
            return 1;
        case lza::pack_result::append_failed:
            if (!warned) fprintf(stderr, "lza-cli: %s: %s\n", result.failed_file.c_str(), lza::pack_error(result.error));
            return 1;
        case lza::pack_result::finish_failed:
            fprintf(stderr, "lza-cli: %s: cannot finalize the archive\n", _opt.archive.c_str());
            return 1;
        }
        return 1;
    }

    struct list_stats {
        uint64_t files{0};
        uint64_t bytes{0};
    };

    void list_entries(const alx::varmap& _map, const std::string& _prefix, list_stats& _stats) {
        for (const auto& it : _map) {
            const std::string name = _prefix.empty() ? it.first : _prefix + "/" + it.first;
            const uint64_t bytes = lza::entry_bytes(*it.second);
            if (it.second->is<alx::varmap>()) {
                printf("%12s  %s/\n", lza::size_string(bytes).c_str(), name.c_str());
                list_entries(it.second->to<alx::varmap>(), name, _stats);
            } else {
                _stats.files++;
                _stats.bytes += bytes;
                printf("%12s  %s\n", lza::size_string(bytes).c_str(), name.c_str());
            }
        }
    }

    int command_list(const options& _opt) {
        lza::open_result open_result;
        const auto archive = lza::archive::open(_opt.archive, _opt.password, open_result);
        if (!archive) return report_open_error(_opt.archive, open_result);

        printf("Listing archive: %s\n\n", _opt.archive.c_str());
        printf("%12s  %s\n", "Size", "Name");
        printf("%12s  %s\n", "------------", "----");

        list_stats stats;
        list_entries(archive->fsystem(), std::string(), stats);
        printf("\n  %llu file%s, %s\n", (unsigned long long) stats.files, stats.files == 1 ? "" : "s", lza::size_string(stats.bytes).c_str());
        return 0;
    }

    int split_entry(const std::string& _item, std::string& _root, std::list<std::string>& _path) {
        std::string current;
        bool first = true;
        for (const char ch : _item) {
            if (ch != '/') {
                current += ch;
                continue;
            }
            if (current.empty()) return -1;
            if (first) {
                _root = current;
                first = false;
            } else _path.push_back(current);
            current.clear();
        }
        if (current.empty()) return -1;
        if (first) _root = current;
        else _path.push_back(current);
        return 0;
    }

    int command_extract(const options& _opt) {
        if (!alx::file_info(_opt.output).is_dir()) {
            fprintf(stderr, "lza-cli: %s: not a directory\n", _opt.output.c_str());
            return 1;
        }

        lza::open_result open_result;
        const auto archive = lza::archive::open(_opt.archive, _opt.password, open_result);
        if (!archive) return report_open_error(_opt.archive, open_result);

        printf("Extracting archive: %s\n", _opt.archive.c_str());
        const auto on_file = [](const std::string& _file, uint64_t _total, uint64_t _current) {
            if (_total != uint64_t(-1) && _total == _current) printf("- %s\n", _file.c_str());
        };

        if (_opt.items.empty()) {
            const auto result = archive->extract("*", std::list<std::string>(), _opt.output, nullptr, on_file, nullptr);
            if (result.state != lza::extract_result::ok) return report_extract_error(result, _opt.archive);
            fputs("Everything is Ok\n", stdout);
            return 0;
        }

        for (const auto& item : _opt.items) {
            std::string root;
            std::list<std::string> path;
            if (split_entry(item, root, path) != 0) {
                fprintf(stderr, "lza-cli: %s: invalid entry path\n", item.c_str());
                return 1;
            }

            const auto result = archive->extract(root, path, _opt.output, nullptr, on_file, nullptr);
            if (result.state != lza::extract_result::ok) return report_extract_error(result, item);
        }
        fputs("Everything is Ok\n", stdout);
        return 0;
    }

}

namespace lza {
    namespace cli {

        int run(int _argc, char** _argv) {
            options opt;
            const int parsed = parse(_argc, _argv, opt);
            if (parsed != 0) return parsed == 1 ? 0 : parsed;

            switch (opt.command) {
            case 'a': return command_add(opt);
            case 'l': return command_list(opt);
            case 'x': return command_extract(opt);
            default: return 2;
            }
        }

    }
}
