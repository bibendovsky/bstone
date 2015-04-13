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


#ifndef BSTONE_CL_ARGS_INCLUDED
#define BSTONE_CL_ARGS_INCLUDED


#include "bstone_string_helper.h"


namespace bstone {


class ClArgs {
public:
    ClArgs();

    ClArgs(
        const ClArgs& that);

    ClArgs& operator=(
        const ClArgs& that);

    ~ClArgs();


    const std::string& operator[](
        int index) const;

    void initialize(
        int argc,
        char* const* argv);

    void unintialize();

    bool has_option(
        const std::string& option_name) const;

    int find_option(
        const std::string& option_name) const;

    int find_argument(
        const std::string& name) const;

    int check_argument(
        const char* const list[]);

    int check_argument(
        const char* const list[],
        std::string& found_argument);

    int get_count() const;

    const std::string& get_argument(
        int index) const;

    const std::string& get_option_value(
        const std::string& option_name) const;

    void get_option_values(
        const std::string& option_name,
        std::string& value1,
        std::string& value2) const;

private:
    StringList args_;
    StringList lc_args_;
}; // class ClArgs


} // namespace bstone


#endif // BSTONE_CL_ARGS_INCLUDED
