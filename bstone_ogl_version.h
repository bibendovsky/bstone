//
// A wrapper for an OpenGL version string.
//


#ifndef BSTONE_OGL_VERSION_H
#define BSTONE_OGL_VERSION_H


#include <string>


namespace bstone {


// A wrapper for an OpenGL version string.
class OglVersion {
public:
    OglVersion(
        int major = 0,
        int minor = 0,
        int release = 0,
        const std::string& vendor = "");

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

    // Builds a string from version's parts.
    std::string to_string() const;

private:
    int major_;
    int minor_;
    int release_;
    std::string vendor_;
    std::string original_;
}; // class OglVersion


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


#endif // BSTONE_OGL_VERSION_H
