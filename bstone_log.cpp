/* ==============================================================
bstone: A source port of Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013 Boris Bendovsky (bibendovsky@hotmail.com)

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
============================================================== */


//
// Logging facility
//


#include "bstone_log.h"
#include <iostream>
#include "SDL.h"


namespace bstone {


Log::Log() :
        fstream_(),
        args_(),
        sstream_(),
        message_(),
        message_type_()
{
    fstream_.open("bstone_log.txt");
    fstream_ << std::unitbuf;

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
Log& Log::get_local()
{
    static Log log;
    static auto is_initialized = false;

    if (!is_initialized) {
        is_initialized = true;

        write("bstone");
        write("======");
        write();
    }

    return log;
}

void Log::write_internal(
    const std::string& format)
{
    auto is_critical = false;

    switch (message_type_) {
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
    fstream_ << message_ << std::endl;

    if (is_critical) {
        static_cast<void>(::SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR,
            "bstone",
            message_.c_str(),
            nullptr));
    }
}


} // bstone
