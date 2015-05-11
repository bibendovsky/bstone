/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2015 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


//
// Logging facility
//


#include "bstone_log.h"
#include <iostream>
#include <stdexcept>
#include "SDL.h"


const std::string& get_version_string();
const std::string& get_profile_dir();


namespace bstone {


Log::Log() :
        fstream_(),
        args_(),
        sstream_(),
        message_(),
        message_type_()
{
    auto log_path = ::get_profile_dir() + "bstone_log.txt";
    fstream_.open(log_path, StreamOpenMode::write);

    args_.reserve(16);
    message_.reserve(1024);
}

Log::~Log()
{
}

// (static)
void Log::write()
{
    write(std::string());
}

// (static)
void Log::write_version()
{
    clean_up();

    write_internal(
        MessageType::version,
        "BStone version: {}",
        ::get_version_string());
}

// (static)
Log& Log::get_local()
{
    static Log log;
    static auto is_initialized = false;

    if (!is_initialized) {
        is_initialized = true;

        write("BStone Log");
        write("==========");
        write();
        write("Version: {}", ::get_version_string());
        write();
    }

    return log;
}

void Log::write_internal(
    const std::string& format)
{
    auto is_critical = false;
    auto is_version = false;

    switch (message_type_) {
    case MessageType::version:
        is_version = true;
        message_.clear();
        break;

    case MessageType::information:
        message_.clear();
        break;

    case MessageType::warning:
        message_ = "WARNING: ";
        break;

    case MessageType::error:
        message_ = "ERROR: ";
        break;

    case MessageType::critical_error:
        is_critical = true;
        message_ = "CRITICAL: ";
        break;

    default:
        throw std::runtime_error("Invalid message type.");
    }

    if (args_.empty()) {
        message_ += format;
    } else if (!format.empty()) {
        auto i = 0;
        auto prev_char = '\0';
        auto c_format = format.c_str();
        auto arg_index = 0;
        while (c_format[i] != '\0') {
            auto ch = c_format[i];
            auto just_advance = false;

            if (prev_char != '{' && ch == '{') {
                int next_char = c_format[i + 1];

                if (next_char == '}') {
                    if (arg_index < static_cast<int>(args_.size())) {
                        message_ += args_[arg_index];

                        i += 2;
                        arg_index += 1;
                    } else {
                        just_advance = true;
                    }
                } else {
                    auto digit = next_char - '0';

                    if (digit >= 0 && digit <= 9) {
                        next_char = c_format[i + 2];

                        if (next_char == '}') {
                            if (digit < static_cast<int>(args_.size())) {
                                message_ += args_[digit];
                                i += 3;
                            } else {
                                just_advance = true;
                            }
                        } else {
                            just_advance = true;
                        }
                    } else {
                        just_advance = true;
                    }
                }
            } else {
                just_advance = true;
            }

            if (just_advance) {
                ++i;
                message_ += ch;
            }

            prev_char = c_format[i - 1];
        }
    }

    std::cout << message_ << std::endl;

    if (!is_version) {
        fstream_.write_string(message_);
        fstream_.write_octet('\n');
    }

    if (is_critical || is_version) {
        static_cast<void>(::SDL_ShowSimpleMessageBox(
            is_version ? SDL_MESSAGEBOX_INFORMATION : SDL_MESSAGEBOX_ERROR,
            "BStone",
            message_.c_str(),
            nullptr));
    }
}

// (static)
void Log::clean_up()
{
    auto&& local = get_local();
    local.args_.clear();
}


} // bstone
