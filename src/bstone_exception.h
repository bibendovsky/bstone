/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// Base exception.
//


#ifndef BSTONE_EXCEPTION_INCLUDED
#define BSTONE_EXCEPTION_INCLUDED


#include <deque>
#include <exception>
#include <memory>
#include <string>


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class Exception : public std::exception
{
public:
	Exception(const char* context, const char* message) noexcept;
	Exception(const Exception& rhs) noexcept;
	~Exception() override;

	const char* what() const noexcept override;

private:
	using What = std::unique_ptr<char[]>;

	What what_{};
}; // Exception

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

using ExceptionMessages = std::deque<std::string>;

ExceptionMessages extract_exception_messages();

std::string get_nested_message();

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone


#endif // !BSTONE_EXCEPTION_INCLUDED
