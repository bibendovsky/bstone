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
// A wrapper for an OpenGL version string.
//


#ifndef BSTONE_OGL_VERSION_INCLUDED
#define BSTONE_OGL_VERSION_INCLUDED


#include <string>


namespace bstone {


// A wrapper for an OpenGL version string.
class OglVersion {
public:
    explicit OglVersion(
        int major = 0,
        int minor = 0,
        int release = 0,
        bool is_es = false);

    OglVersion(
        const std::string& version_string);

    OglVersion(
        const OglVersion& that);

    OglVersion& operator=(
        const OglVersion& that);

    // Parses version information from a string.
    void set(
        const std::string& version_string);

    // Clears version info.
    void reset();

    // Returns a major part of the version.
    int get_major() const;

    // Returns a minor part of the version.
    int get_minor() const;

    // Returns a release part of the version.
    int get_release() const;

    // Returns a vendor part of the version.
    const std::string& get_vendor() const;

    // Returns an original version string.
    const std::string& get_original() const;

    // Returns true if it's OpenGL ES implementation or
    // false otherwise.
    bool is_es() const;

    // Builds a string from version's parts.
    std::string to_string() const;

private:
    int major_;
    int minor_;
    int release_;
    std::string vendor_;
    std::string original_;
    bool is_es_;

    const std::string& get_es_prefix() const;
}; // OglVersion


} // namespace bstone


bool operator<(
    const bstone::OglVersion& a,
    const bstone::OglVersion& b);

bool operator>(
    const bstone::OglVersion& a,
    const bstone::OglVersion& b);

bool operator<=(
    const bstone::OglVersion& a,
    const bstone::OglVersion& b);

bool operator>=(
    const bstone::OglVersion& a,
    const bstone::OglVersion& b);

bool operator==(
    const bstone::OglVersion& a,
    const bstone::OglVersion& b);

bool operator!=(
    const bstone::OglVersion& a,
    const bstone::OglVersion& b);


#endif // BSTONE_OGL_VERSION_INCLUDED
