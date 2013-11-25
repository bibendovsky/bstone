#ifndef BSTONE_CL_ARGS_H
#define BSTONE_CL_ARGS_H


#include "bstone_string_helper.h"


namespace bstone {


class ClArgs {
public:
    ClArgs();

    ClArgs(
        const ClArgs& that);

    ~ClArgs();

    ClArgs& operator=(
        const ClArgs& that);

    const std::string& operator[](
        int index) const;

    void initialize(
        int argc,
        char* const* argv);

    void unintialize();

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


#endif // BSTONE_CL_ARGS_H
