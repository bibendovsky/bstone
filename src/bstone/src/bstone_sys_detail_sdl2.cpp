/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_exception.h"
#include "bstone_sys_detail_sdl2.h"

namespace bstone {
namespace sys {
namespace detail {

void sdl2_log_eol(std::string& message)
{
	message += '\n';
}

void sdl2_log_bool(bool value, std::string& message)
{
	value ? message += "true" : message += "false";
}

void sdl2_log_gl_attributes(const GlContextAttributes& gl_attribs, std::string& message)
{
	// is accelerated
	//
	message += "    Accelerated: ";
	sdl2_log_bool(gl_attribs.is_accelerated, message);
	sdl2_log_eol(message);

	// profile
	//
	message += "    Profile: ";
	
	switch (gl_attribs.profile)
	{
		case GlContextProfile::none: message += "none"; break;
		case GlContextProfile::compatibility: message += "compatibility"; break;
		case GlContextProfile::core: message += "core"; break;
		case GlContextProfile::es: message += "es"; break;
		default: message += "???"; break;
	}

	sdl2_log_eol(message);

	// major version
	//
	message += "    Major version: ";
	sdl2_log_xint(gl_attribs.major_version, message);
	sdl2_log_eol(message);

	// minor version
	//
	message += "    Minor version: ";
	sdl2_log_xint(gl_attribs.minor_version, message);
	sdl2_log_eol(message);

	// multisample buffer count
	//
	message += "    Multisample buffer count: ";
	sdl2_log_xint(gl_attribs.multisample_buffer_count, message);
	sdl2_log_eol(message);

	// multisample sample count
	//
	message += "    Multisample sample count: ";
	sdl2_log_xint(gl_attribs.multisample_sample_count, message);
	sdl2_log_eol(message);

	// red bit count
	//
	message += "    Red bit count: ";
	sdl2_log_xint(gl_attribs.red_bit_count, message);
	sdl2_log_eol(message);

	// green bit count
	//
	message += "    Green bit count: ";
	sdl2_log_xint(gl_attribs.green_bit_count, message);
	sdl2_log_eol(message);

	// blue bit count
	//
	message += "    Blue bit count: ";
	sdl2_log_xint(gl_attribs.blue_bit_count, message);
	sdl2_log_eol(message);

	// alpha bit count
	//
	message += "    Alpha bit count: ";
	sdl2_log_xint(gl_attribs.alpha_bit_count, message);
	sdl2_log_eol(message);

	// depth bit count
	//
	message += "    Depth bit count: ";
	sdl2_log_xint(gl_attribs.depth_bit_count, message);
	sdl2_log_eol(message);
}

} // namespace detail
} // namespace sys
} // namespace bstone
