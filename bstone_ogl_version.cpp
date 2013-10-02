#include "bstone_ogl_version.h"

#include <sstream>


namespace bstone {


OglVersion::OglVersion(
    int major,
    int minor,
    int release,
    const std::string& vendor) :
        major_(major),
        minor_(minor),
        release_(release),
        vendor_(vendor),
        original_()
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
        original_(that.original_)
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
    }

    return *this;
}

void OglVersion::set(
    const std::string& version_string)
{
    reset();

    const int eof = std::istringstream::traits_type::eof();

    int next_char;
    std::istringstream iss(version_string);
    iss.unsetf(std::ios::skipws);

    // Major.
    int major = 0;

    if (!(iss >> major))
        return;

    next_char = iss.peek();

    if (next_char != '.')
        return;

    // Minor.
    int minor = 0;

    iss.ignore(1);

    if (!(iss >> minor))
        return;

    // Release (optional).
    int release = 0;

    next_char = iss.peek();

    if (next_char != eof && next_char == '.') {
        iss.ignore(1);

        if (!(iss >> release))
            return;

        next_char = iss.peek();
    }

    // Vendor (optional)
    std::string vendor;

    if (next_char != eof && next_char == ' ') {
        iss.ignore(1);
        std::getline(iss, vendor);

        if (!iss)
            return;
    }

    if (!iss.eof())
        return;

    major_ = major;
    minor_ = minor;
    release_ = release;
    vendor_ = vendor;
    original_ = version_string;
}

void OglVersion::reset()
{
    major_ = 0;
    minor_ = 0;
    release_ = 0;
    vendor_.clear();
    original_.clear();
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

std::string OglVersion::to_string() const
{
    std::ostringstream oss;

    oss << get_major() << '.' << get_minor();

    if (get_release() != 0)
        oss << '.' << get_release();

    if (!get_vendor().empty())
        oss << ' ' << get_vendor();

    return oss.str();
}


} // namespace bstone


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
