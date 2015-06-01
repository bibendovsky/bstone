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


#include "bstone_ogl_version.h"
#include <sstream>


namespace bstone {


OglVersion::OglVersion(
    int major,
    int minor,
    int release,
    bool is_gles) :
        major_(major),
        minor_(minor),
        release_(release),
        is_es_(is_gles)
{
}

OglVersion::OglVersion(
    const std::string& version_string)
{
    set(version_string);
}

OglVersion::OglVersion(
    const OglVersion& that) :
        major_(that.major_),
        minor_(that.minor_),
        release_(that.release_),
        vendor_(that.vendor_),
        original_(that.original_),
        is_es_(that.is_es_)
{
}

OglVersion& OglVersion::operator=(
    const OglVersion& that)
{
    if (&that != this) {
        major_ = that.major_;
        minor_ = that.minor_;
        release_ = that.release_;
        vendor_ = that.vendor_;
        original_ = that.original_;
        is_es_ = that.is_es_;
    }

    return *this;
}

void OglVersion::set(
    const std::string& version_string)
{
    reset();

    const auto eof = std::istringstream::traits_type::eof();

    int next_char;
    std::istringstream iss(version_string);
    iss.unsetf(std::ios::skipws);

    // Check for OpenGL ES.
    bool is_gles = false;

    if (version_string.find(get_es_prefix()) == 0) {
        is_gles = true;
        iss.ignore(get_es_prefix().size());
    }

    // Major.
    int major = 0;

    if (!(iss >> major)) {
        return;
    }

    next_char = iss.peek();

    if (next_char != '.') {
        return;
    }

    // Minor.
    int minor = 0;

    iss.ignore(1);

    if (!(iss >> minor)) {
        return;
    }

    // Release (optional).
    int release = 0;

    next_char = iss.peek();

    if (next_char != eof && next_char == '.') {
        iss.ignore(1);

        if (!(iss >> release)) {
            return;
        }

        next_char = iss.peek();
    }

    // Vendor (optional)
    std::string vendor;

    if (next_char != eof && next_char == ' ') {
        iss.ignore(1);
        std::getline(iss, vendor);

        if (!iss) {
            return;
        }
    }

    if (!iss.eof()) {
        return;
    }

    major_ = major;
    minor_ = minor;
    release_ = release;
    vendor_ = vendor;
    original_ = version_string;
    is_es_ = is_gles;
}

void OglVersion::reset()
{
    major_ = 0;
    minor_ = 0;
    release_ = 0;
    vendor_.clear();
    original_.clear();
    is_es_ = false;
}

int OglVersion::get_major() const
{
    return major_;
}

int OglVersion::get_minor() const
{
    return minor_;
}

int OglVersion::get_release() const
{
    return release_;
}

const std::string& OglVersion::get_vendor() const
{
    return vendor_;
}

const std::string& OglVersion::get_original() const
{
    return original_;
}

bool OglVersion::is_es() const
{
    return is_es_;
}

std::string OglVersion::to_string() const
{
    std::ostringstream oss;

    if (is_es()) {
        oss << get_es_prefix();
    }

    oss << get_major() << '.' << get_minor();

    if (get_release() != 0) {
        oss << '.' << get_release();
    }

    if (!get_vendor().empty()) {
        oss << ' ' << get_vendor();
    }

    return oss.str();
}

const std::string& OglVersion::get_es_prefix() const
{
    static std::string result = "OpenGL ES ";
    return result;
}


} // bstone


bool operator<(
    const bstone::OglVersion& a,
    const bstone::OglVersion& b)
{
    return
        a.get_major() < b.get_major() ||
        a.get_minor() < b.get_minor() ||
        a.get_release() < b.get_release();
}

bool operator>(
    const bstone::OglVersion& a,
    const bstone::OglVersion& b)
{
    return
        a.get_major() > b.get_major() ||
        a.get_minor() > b.get_minor() ||
        a.get_release() > b.get_release();
}

bool operator<=(
    const bstone::OglVersion& a,
    const bstone::OglVersion& b)
{
    return !(a > b);
}

bool operator>=(
    const bstone::OglVersion& a,
    const bstone::OglVersion& b)
{
    return !(a < b);
}

bool operator==(
    const bstone::OglVersion& a,
    const bstone::OglVersion& b)
{
    return
        a.get_major() == b.get_major() &&
        a.get_minor() == b.get_minor() &&
        a.get_release() == b.get_release();
}

bool operator!=(
    const bstone::OglVersion& a,
    const bstone::OglVersion& b)
{
    return !(a == b);
}
