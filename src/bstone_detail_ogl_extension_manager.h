/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2019 Boris I. Bendovsky (bibendovsky@hotmail.com)

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


//
// OpenGL extension manager (implementation).
//
// !!! Internal usage only !!!
//


#ifndef BSTONE_DETAIL_OGL_EXTENSION_MANAGER_INCLUDED
#define BSTONE_DETAIL_OGL_EXTENSION_MANAGER_INCLUDED


#include <memory>
#include <string>


namespace bstone
{
namespace detail
{


enum class OglExtensionId
{
	// Virtual extensions.
	//

	// OpenGL 1.0.
	v1_0,

	// OpenGL 1.1.
	v1_1,

	// OpenGL 1.2.
	v1_2,

	// OpenGL 1.3.
	v1_3,

	// OpenGL 1.4.
	v1_4,

	// OpenGL 1.5.
	v1_5,

	// OpenGL 2.0.
	v2_0,

	// OpenGL 2.1.
	v2_1,

	// OpenGL 3.0.
	v3_0,

	// OpenGL 3.1.
	v3_1,


	// Real extensions.
	//

	arb_color_buffer_float,
	arb_copy_buffer,
	arb_depth_buffer_float,
	arb_framebuffer_object,
	arb_framebuffer_srgb,
	arb_half_float_vertex,
	arb_map_buffer_range,
	arb_texture_compression,
	arb_texture_compression_rgtc,
	arb_texture_cube_map,
	arb_texture_filter_anisotropic,
	arb_texture_float,
	arb_texture_non_power_of_two,
	arb_texture_rg,
	arb_uniform_buffer_object,
	arb_vertex_array_object,
	arb_vertex_buffer_object,

	ext_framebuffer_blit,
	ext_framebuffer_multisample,
	ext_framebuffer_object,
	ext_packed_depth_stencil,
	ext_texture,
	ext_texture_filter_anisotropic,

	sgis_generate_mipmap,


	// Item count.
	count_,
}; // OglExtensionId


class OglExtensionManager
{
protected:
	OglExtensionManager();


public:
	virtual ~OglExtensionManager();


	virtual int get_extension_count() const = 0;

	virtual const std::string& get_extension_name(
		const int extension_index) const = 0;


	virtual void probe_extension(
		const OglExtensionId extension_id) = 0;


	virtual bool has_extension(
		const OglExtensionId extension_id) const = 0;

	virtual bool operator[](
		const OglExtensionId extension_id) const = 0;
}; // OglExtensionManager

using OglExtensionManagerPtr = OglExtensionManager*;
using OglExtensionManagerUPtr = std::unique_ptr<OglExtensionManager>;


struct OglExtensionManagerFactory
{
	static OglExtensionManagerUPtr create();
}; // OglExtensionManagerFactory


} // detail
} // bstone


#endif // !BSTONE_DETAIL_OGL_EXTENSION_MANAGER_INCLUDED
