#include "bstone_cl_args.h"


namespace bstone {


ClArgs::ClArgs()
{
}

ClArgs::ClArgs(
    const ClArgs& that) :
        args_(that.args_),
        lc_args_(that.lc_args_)
{
}

ClArgs::~ClArgs()
{
}

ClArgs& ClArgs::operator=(
    const ClArgs& that)
{
    if (&that != this) {
        args_ = that.args_;
        lc_args_ = that.lc_args_;
    }

    return *this;
}

const std::string& ClArgs::operator[](
    int index) const
{
    return args_[index];
}

void ClArgs::initialize(
    int argc,
    char* const* argv)
{
    unintialize();

    args_.resize(argc);
    lc_args_.resize(argc);

    for (int i = 0; i < argc; ++i) {
        args_[i] = argv[i];
        lc_args_[i] = StringHelper::to_lower(args_.back());
    }
}

void ClArgs::unintialize()
{
    StringList().swap(args_);
    StringList().swap(lc_args_);
}

int ClArgs::find_option(
    const std::string& option_name) const
{
    if (option_name.empty())
        return -1;

    std::string lc_name = StringHelper::to_lower(option_name);

    for (int i = 1; i < get_count(); ++i) {
        const std::string& arg = args_[i];

        if (arg.size() != (2 + lc_name.size()))
            continue;

        if (arg.compare(0, 2, "--") != 0)
            continue;

        if (arg.compare(2, lc_name.size(), lc_name) == 0)
            return i;
    }

    return -1;
}

int ClArgs::find_argument(
    const std::string& name) const
{
    if (name.empty())
        return -1;

    std::string lc_name = StringHelper::to_lower(name);

    for (int i = 1; i < get_count(); ++i) {
        if (name == args_[i])
            return i;
    }

    return -1;
}

int ClArgs::check_argument(
    const char* const list[])
{
    if (args_.size () <= 1)
        return -1;

    if (list == NULL)
        return -1;

    std::string item;

    for (StringListCIt i = lc_args_.begin() + 1; i != lc_args_.end();
        ++i)
    {
        size_t arg_index = 0;

        if (i->size() >= 2 && i->compare(0, 2, "--") == 0)
            arg_index = 2;

        item = StringHelper::to_lower(i->substr(arg_index));

        if (item.empty())
            continue;

        for (int i = 0; list[i] != NULL; ++i) {
            std::string list_item = list[i];

            if (item == list_item)
                return i;
        }
    }

    return -1;
}

int ClArgs::check_argument(
    const char* const list[],
    std::string& found_argument)
{
    found_argument.clear();

    if (args_.size () <= 1)
        return -1;

    if (list == NULL)
        return -1;

    std::string item;

    for (StringListCIt i = lc_args_.begin() + 1; i != lc_args_.end();
        ++i)
    {
        size_t arg_index = 0;

        if (i->size() >= 2 && i->compare(0, 2, "--") == 0)
            arg_index = 2;

        item = StringHelper::to_lower(i->substr(arg_index));

        if (item.empty())
            continue;

        for (int i = 0; list[i] != NULL; ++i) {
            std::string list_item = list[i];

            if (item == list_item) {
                found_argument = item;
                return i;
            }
        }
    }

    return -1;
}

int ClArgs::get_count() const
{
    return static_cast<int>(args_.size());
}


} // namespace bstone
