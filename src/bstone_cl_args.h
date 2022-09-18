/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


#ifndef BSTONE_CL_ARGS_INCLUDED
#define BSTONE_CL_ARGS_INCLUDED


#include <string>
#include <vector>


namespace bstone
{


class ClArgs
{
public:
	ClArgs();


	const std::string& operator[](
		const int index) const;

	void initialize(
		const int argc,
		char* const* argv);

	void unintialize();

	bool has_option(
		const std::string& option_name) const;

	int find_option(
		const std::string& option_name) const;

	int find_argument(
		const std::string& name) const;

	int get_count() const;

	const std::string& get_argument(
		const int index) const;

	const std::string& get_option_value(
		const std::string& option_name) const;

	void get_option_values(
		const std::string& option_name,
		std::string& value1,
		std::string& value2) const;


private:
	using StringList = std::vector<std::string>;


	StringList args_;
	StringList lc_args_;
}; // ClArgs


} // bstone


#endif // !BSTONE_CL_ARGS_INCLUDED
