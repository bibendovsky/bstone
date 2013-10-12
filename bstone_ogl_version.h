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
