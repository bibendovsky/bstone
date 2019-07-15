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


#include "bstone_precompiled.h"
#include "bstone_detail_ogl_extension_manager.h"
#include <cassert>
#include <algorithm>
#include <iterator>
#include <sstream>
#include "bstone_detail_ogl_renderer_utils.h"
#include "bstone_sdl_types.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// OglExtensionManagerImpl
//

class OglExtensionManagerImpl :
	public OglExtensionManager
{
public:
	OglExtensionManagerImpl();

	OglExtensionManagerImpl(
		const OglExtensionManagerImpl& rhs) = delete;

	OglExtensionManagerImpl(
		OglExtensionManagerImpl&& rhs);

	~OglExtensionManagerImpl() override;


	int get_extension_count() const override;

	const std::string& get_extension_name(
		const int extension_index) const override;


	void probe_extension(
		const OglExtensionId extension_id) override;


	bool has_extension(
		const OglExtensionId extension_id) const override;

	bool operator[](
		const OglExtensionId extension_id) const override;


	bool initialize();


private:
	using ExtensionNames = std::vector<std::string>;

	using Dependencies = std::vector<OglExtensionId>;
	using ResolveSymbolsFunction = bool (OglExtensionManagerImpl::*)();


	struct RegistryItem
	{
		bool is_virtual_;
		bool is_probed_;
		bool is_available_;
		bool is_gl_;
		bool is_glcore_;
		bool is_gles1_;
		bool is_gles2_;

		std::string extension_name_;
		ResolveSymbolsFunction resolve_symbols_function_;
		Dependencies dependencies_;
	}; // RegistryItem

	using Registry = std::vector<RegistryItem>;


	OglRendererUtilsContextType context_type_;
	int context_major_version_;
	int context_minor_version_;
	ExtensionNames extension_names_;
	Registry registry_;


	static const std::string& get_empty_extension_name();

	static int get_registered_extension_count();

	static int get_extension_index(
		const OglExtensionId extension_id);


	template<typename T>
	static void resolve_symbol(
		const char* const name,
		T& symbol,
		bool& is_failed)
	{
		symbol = reinterpret_cast<T>(OglRendererUtils::resolve_symbol(name));

		if (symbol == nullptr)
		{
			is_failed = true;
		}
	}


	void initialize_registry();


	bool get_context_attributes();


	void get_core_extension_names();

	void get_compatibility_extension_names();

	void get_extension_names();


	bool is_gl() const;

	bool is_glcore() const;

	bool is_gles1() const;

	bool is_gles2() const;

	bool is_extension_compatible(
		const OglExtensionId extension_id) const;


	void probe_generic(
		const OglExtensionId extension_id);


	bool resolve_v1_0();

	bool resolve_v1_1();

	bool resolve_v1_2();

	bool resolve_v1_3();

	bool resolve_v1_4();

	bool resolve_v1_5();


	bool resolve_v2_0();

	bool resolve_v2_1();


	bool resolve_v3_0();

	bool resolve_v3_1();


	bool resolve_arb_color_buffer_float();

	bool resolve_arb_copy_buffer();

	bool resolve_arb_framebuffer_object();

	bool resolve_arb_map_buffer_range();

	bool resolve_arb_uniform_buffer_object();

	bool resolve_arb_vertex_array_object();

	bool resolve_arb_vertex_buffer_object();


	bool resolve_ext_framebuffer_blit();

	bool resolve_ext_framebuffer_multisample();

	bool resolve_ext_framebuffer_object();
}; // OglExtensionManagerImpl


using OglExtensionManagerImplPtr = OglExtensionManagerImpl*;
using OglExtensionManagerImplUPtr = std::unique_ptr<OglExtensionManagerImpl>;


OglExtensionManagerImpl::OglExtensionManagerImpl()
	:
	context_type_{},
	extension_names_{},
	registry_{}
{
}

OglExtensionManagerImpl::OglExtensionManagerImpl(
	OglExtensionManagerImpl&& rhs)
	:
	context_type_{std::move(rhs.context_type_)},
	extension_names_{std::move(rhs.extension_names_)},
	registry_{std::move(rhs.registry_)}
{
}

OglExtensionManagerImpl::~OglExtensionManagerImpl()
{
}

bool OglExtensionManagerImpl::initialize()
{
	if (!get_context_attributes())
	{
		assert(!"Failed to get context's attributes.");

		return false;
	}

	get_extension_names();

	initialize_registry();

	return true;
}

int OglExtensionManagerImpl::get_extension_count() const
{
	return static_cast<int>(extension_names_.size());
}

const std::string& OglExtensionManagerImpl::get_extension_name(
	const int extension_index) const
{
	if (extension_index < 0 || extension_index >= get_extension_count())
	{
		return get_empty_extension_name();
	}

	return extension_names_[extension_index];
}

void OglExtensionManagerImpl::probe_extension(
	const OglExtensionId extension_id)
{
	probe_generic(extension_id);
}

bool OglExtensionManagerImpl::has_extension(
	const OglExtensionId extension_id) const
{
	const auto extension_index = get_extension_index(extension_id);

	if (extension_index < 0)
	{
		return false;
	}

	return registry_[extension_index].is_available_;
}

bool OglExtensionManagerImpl::operator[](
	const OglExtensionId extension_id) const
{
	return has_extension(extension_id);
}

const std::string& OglExtensionManagerImpl::get_empty_extension_name()
{
	static const auto result = std::string{};

	return result;
}

int OglExtensionManagerImpl::get_registered_extension_count()
{
	return static_cast<int>(OglExtensionId::count_);
}

int OglExtensionManagerImpl::get_extension_index(
	const OglExtensionId extension_id)
{
	const auto extension_index = static_cast<int>(extension_id);

	if (extension_index < 0 || extension_index >= get_registered_extension_count())
	{
		return -1;
	}

	return extension_index;
}

void OglExtensionManagerImpl::initialize_registry()
{
	registry_.clear();
	registry_.resize(static_cast<int>(OglExtensionId::count_));

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::v1_0)];
		registry_item.is_virtual_ = true;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = false;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "v1.0";
		registry_item.resolve_symbols_function_ = &OglExtensionManagerImpl::resolve_v1_0;
		registry_item.dependencies_ = {};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::v1_1)];
		registry_item.is_virtual_ = true;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = false;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "v1.1";
		registry_item.resolve_symbols_function_ = &OglExtensionManagerImpl::resolve_v1_1;
		registry_item.dependencies_ = {OglExtensionId::v1_0};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::v1_2)];
		registry_item.is_virtual_ = true;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = false;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "v1.2";
		registry_item.resolve_symbols_function_ = &OglExtensionManagerImpl::resolve_v1_2;
		registry_item.dependencies_ = {OglExtensionId::v1_1};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::v1_3)];
		registry_item.is_virtual_ = true;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = false;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "v1.3";
		registry_item.resolve_symbols_function_ = &OglExtensionManagerImpl::resolve_v1_3;
		registry_item.dependencies_ = {OglExtensionId::v1_2};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::v1_4)];
		registry_item.is_virtual_ = true;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = false;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "v1.4";
		registry_item.resolve_symbols_function_ = &OglExtensionManagerImpl::resolve_v1_4;
		registry_item.dependencies_ = {OglExtensionId::v1_3};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::v1_5)];
		registry_item.is_virtual_ = true;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = false;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "v1.5";
		registry_item.resolve_symbols_function_ = &OglExtensionManagerImpl::resolve_v1_5;
		registry_item.dependencies_ = {OglExtensionId::v1_4};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::v2_0)];
		registry_item.is_virtual_ = true;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = false;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "v2.0";
		registry_item.resolve_symbols_function_ = &OglExtensionManagerImpl::resolve_v2_0;
		registry_item.dependencies_ = {OglExtensionId::v1_5};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::v2_1)];
		registry_item.is_virtual_ = true;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = false;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "v2.1";
		registry_item.resolve_symbols_function_ = &OglExtensionManagerImpl::resolve_v2_1;
		registry_item.dependencies_ = {OglExtensionId::v2_0};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::v3_0)];
		registry_item.is_virtual_ = true;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = false;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "v3.0";
		registry_item.resolve_symbols_function_ = &OglExtensionManagerImpl::resolve_v3_0;

		registry_item.dependencies_ =
		{
			OglExtensionId::arb_depth_buffer_float,
			OglExtensionId::arb_framebuffer_object,
			OglExtensionId::arb_texture_float,
			OglExtensionId::arb_framebuffer_srgb,
			OglExtensionId::arb_half_float_vertex,
			OglExtensionId::arb_map_buffer_range,
			OglExtensionId::arb_texture_compression_rgtc,
			OglExtensionId::arb_texture_rg,
			OglExtensionId::arb_vertex_array_object,
		};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::v3_1)];
		registry_item.is_virtual_ = true;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = false;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "v3.1";
		registry_item.resolve_symbols_function_ = &OglExtensionManagerImpl::resolve_v3_1;

		registry_item.dependencies_ =
		{
			OglExtensionId::v3_0,
			OglExtensionId::arb_copy_buffer,
			OglExtensionId::arb_uniform_buffer_object,
		};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::arb_color_buffer_float)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = true;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "GL_ARB_color_buffer_float";
		registry_item.resolve_symbols_function_ = &OglExtensionManagerImpl::resolve_arb_color_buffer_float;
		registry_item.dependencies_ = {};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::arb_copy_buffer)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = true;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "GL_ARB_copy_buffer";
		registry_item.resolve_symbols_function_ = &OglExtensionManagerImpl::resolve_arb_copy_buffer;
		registry_item.dependencies_ = {};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::arb_depth_buffer_float)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = true;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "GL_ARB_depth_buffer_float";
		registry_item.resolve_symbols_function_ = nullptr;
		registry_item.dependencies_ = {};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::arb_framebuffer_object)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = true;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "GL_ARB_framebuffer_object";
		registry_item.resolve_symbols_function_ = &OglExtensionManagerImpl::resolve_arb_framebuffer_object;
		registry_item.dependencies_ = {};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::arb_framebuffer_srgb)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = false;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "GL_ARB_framebuffer_sRGB";
		registry_item.resolve_symbols_function_ = nullptr;
		registry_item.dependencies_ = {};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::arb_half_float_vertex)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = true;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "GL_ARB_half_float_vertex";
		registry_item.resolve_symbols_function_ = nullptr;
		registry_item.dependencies_ = {};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::arb_map_buffer_range)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = true;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "GL_ARB_map_buffer_range";
		registry_item.resolve_symbols_function_ = &OglExtensionManagerImpl::resolve_arb_map_buffer_range;
		registry_item.dependencies_ = {};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::arb_texture_compression)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = false;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "GL_ARB_texture_compression";
		registry_item.resolve_symbols_function_ = nullptr;
		registry_item.dependencies_ = {};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::arb_texture_compression_rgtc)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = true;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "GL_ARB_texture_compression_rgtc";
		registry_item.resolve_symbols_function_ = nullptr;
		registry_item.dependencies_ = {};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::arb_texture_cube_map)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = false;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "GL_ARB_texture_cube_map";
		registry_item.resolve_symbols_function_ = nullptr;
		registry_item.dependencies_ = {};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::arb_texture_filter_anisotropic)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = true;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "GL_ARB_texture_filter_anisotropic";
		registry_item.resolve_symbols_function_ = nullptr;
		registry_item.dependencies_ = {};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::arb_texture_float)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = false;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "GL_ARB_texture_float";
		registry_item.resolve_symbols_function_ = nullptr;
		registry_item.dependencies_ = {};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::arb_texture_non_power_of_two)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = true;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "GL_ARB_texture_non_power_of_two";
		registry_item.resolve_symbols_function_ = nullptr;
		registry_item.dependencies_ = {};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::arb_texture_rg)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = true;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "GL_ARB_texture_rg";
		registry_item.resolve_symbols_function_ = nullptr;
		registry_item.dependencies_ = {};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::arb_uniform_buffer_object)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = true;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "GL_ARB_uniform_buffer_object";
		registry_item.resolve_symbols_function_ = &OglExtensionManagerImpl::resolve_arb_uniform_buffer_object;
		registry_item.dependencies_ = {};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::arb_vertex_array_object)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = true;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "GL_ARB_vertex_array_object";
		registry_item.resolve_symbols_function_ = &OglExtensionManagerImpl::resolve_arb_vertex_array_object;
		registry_item.dependencies_ = {};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::arb_vertex_buffer_object)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = false;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "GL_ARB_vertex_buffer_object";
		registry_item.resolve_symbols_function_ = &OglExtensionManagerImpl::resolve_arb_vertex_buffer_object;
		registry_item.dependencies_ = {};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::ext_framebuffer_blit)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = false;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "GL_EXT_framebuffer_blit";
		registry_item.resolve_symbols_function_ = &OglExtensionManagerImpl::resolve_ext_framebuffer_blit;
		registry_item.dependencies_ = {};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::ext_framebuffer_multisample)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = false;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "GL_EXT_framebuffer_multisample";
		registry_item.resolve_symbols_function_ = &OglExtensionManagerImpl::resolve_ext_framebuffer_multisample;
		registry_item.dependencies_ = {};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::ext_packed_depth_stencil)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = false;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "GL_EXT_packed_depth_stencil";
		registry_item.resolve_symbols_function_ = nullptr;
		registry_item.dependencies_ = {};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::ext_texture_filter_anisotropic)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = false;
		registry_item.is_gles1_ = true;
		registry_item.is_gles2_ = true;
		registry_item.extension_name_ = "GL_EXT_texture_filter_anisotropic";
		registry_item.resolve_symbols_function_ = nullptr;
		registry_item.dependencies_ = {};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::ext_texture)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = false;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "GL_EXT_texture";
		registry_item.resolve_symbols_function_ = nullptr;
		registry_item.dependencies_ = {};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::sgis_generate_mipmap)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = false;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "GL_SGIS_generate_mipmap";
		registry_item.resolve_symbols_function_ = nullptr;
		registry_item.dependencies_ = {};
	}
}

bool OglExtensionManagerImpl::get_context_attributes()
{
	const auto context_type = OglRendererUtils::context_get_type();

	if (context_type == OglRendererUtilsContextType::invalid)
	{
		return false;
	}

	if (!OglRendererUtils::context_get_version(context_major_version_, context_minor_version_))
	{
		return false;
	}

	int major_version = 0;
	int minor_version = 0;

	if (!OglRendererUtils::context_get_version(major_version, minor_version))
	{
		return false;
	}

	context_type_ = context_type;
	context_major_version_ = major_version;
	context_minor_version_ = minor_version;

	return true;
}

void OglExtensionManagerImpl::get_core_extension_names()
{
	auto is_failed = false;
	auto gl_get_integer_v = PFNGLGETINTEGERVPROC{};
	auto gl_get_string_i = PFNGLGETSTRINGIPROC{};

	resolve_symbol("glGetIntegerv", gl_get_integer_v, is_failed);
	resolve_symbol("glGetStringi", gl_get_string_i, is_failed);

	if (is_failed)
	{
		assert(!"Failed to resolve essential symbols.");

		return;
	}

	auto ogl_extension_count = GLint{};

	gl_get_integer_v(GL_NUM_EXTENSIONS, &ogl_extension_count);

	if (ogl_extension_count == 0)
	{
		return;
	}

	auto extension_names = ExtensionNames{};
	extension_names.resize(ogl_extension_count);

	for (auto i = GLint{}; i < ogl_extension_count; ++i)
	{
		const auto extension_name = gl_get_string_i(GL_EXTENSIONS, static_cast<GLuint>(i));

		if (extension_name == nullptr)
		{
			assert(!"Null extension name.");

			return;
		}

		extension_names[i] = reinterpret_cast<const char*>(extension_name);
	}

	std::swap(extension_names_, extension_names);
}

void OglExtensionManagerImpl::get_compatibility_extension_names()
{
	auto is_failed = false;
	auto gl_get_string = PFNGLGETSTRINGPROC{};

	resolve_symbol("glGetString", gl_get_string, is_failed);

	if (is_failed)
	{
		assert(!"Failed to resolve essential symbols.");

		return;
	}

	const auto ogl_extensions_c_string = gl_get_string(GL_EXTENSIONS);

	if (ogl_extensions_c_string == nullptr)
	{
		assert(!"Null extensions string.");

		return;
	}

	const auto& ogl_extensions_std_string = std::string
	{
		reinterpret_cast<const char*>(ogl_extensions_c_string)
	};

	const auto extension_count = 1 + std::count(
		ogl_extensions_std_string.cbegin(),
		ogl_extensions_std_string.cend(),
		' '
	);

	auto iss = std::istringstream{ogl_extensions_std_string};

	extension_names_.reserve(extension_count);

	extension_names_.assign(
		std::istream_iterator<std::string>{iss},
		std::istream_iterator<std::string>{}
	);
}

void OglExtensionManagerImpl::get_extension_names()
{
	auto is_core = false;

	if (context_type_ == OglRendererUtilsContextType::es)
	{
		if (context_major_version_ >= 3)
		{
			is_core = true;
		}
	}
	else if (context_type_ == OglRendererUtilsContextType::core)
	{
		is_core = true;
	}

	if (is_core)
	{
		get_core_extension_names();
	}
	else
	{
		get_compatibility_extension_names();
	}

	std::sort(extension_names_.begin(), extension_names_.end());
}

bool OglExtensionManagerImpl::is_gl() const
{
	return
		context_type_ == OglRendererUtilsContextType::none ||
		context_type_ == OglRendererUtilsContextType::compatibility
	;
}

bool OglExtensionManagerImpl::is_glcore() const
{
	return
		context_type_ == OglRendererUtilsContextType::core
	;

}

bool OglExtensionManagerImpl::is_gles1() const
{
	return
		context_type_ == OglRendererUtilsContextType::es &&
		context_major_version_ == 1
	;
}

bool OglExtensionManagerImpl::is_gles2() const
{
	return
		context_type_ == OglRendererUtilsContextType::es &&
		context_major_version_ == 2
	;
}

bool OglExtensionManagerImpl::is_extension_compatible(
	const OglExtensionId extension_id) const
{
	const auto extension_index = get_extension_index(extension_id);

	if (extension_index < 0)
	{
		return false;
	}

	const auto& registry_item = registry_[extension_index];

	return
		is_gl() == registry_item.is_gl_ ||
		is_glcore() == registry_item.is_glcore_ ||
		is_gles1() == registry_item.is_gles1_ ||
		is_gles2() == registry_item.is_gles2_
	;
}

void OglExtensionManagerImpl::probe_generic(
	const OglExtensionId extension_id)
{
	const auto extension_index = get_extension_index(extension_id);

	if (extension_index < 0)
	{
		assert(!"Invalid extension id.");

		return;
	}

	auto& registry_item = registry_[extension_index];

	if (registry_item.is_probed_)
	{
		return;
	}

	registry_item.is_probed_ = true;

	if (!is_extension_compatible(extension_id))
	{
		return;
	}

	if (registry_item.is_virtual_ && registry_item.resolve_symbols_function_ == nullptr)
	{
		assert(!"Expected symbols loader for specific version.");

		return;
	}

	for (const auto dependency_extension_id : registry_item.dependencies_)
	{
		if (!is_extension_compatible(dependency_extension_id))
		{
			continue;
		}

		probe_generic(dependency_extension_id);

		if (!has_extension(dependency_extension_id))
		{
			return;
		}
	}

	if (!registry_item.is_virtual_)
	{
		const auto has_extension_name = std::any_of(
			extension_names_.cbegin(),
			extension_names_.cend(),
			[&](const auto& item)
			{
				return item == registry_item.extension_name_;
			}
		);

		if (!has_extension_name)
		{
			return;
		}
	}

	if (registry_item.resolve_symbols_function_ != nullptr)
	{
		if (!(this->*registry_item.resolve_symbols_function_)())
		{
			return;
		}
	}

	registry_item.is_available_ = true;
}

bool OglExtensionManagerImpl::resolve_v1_0()
{
	auto is_failed = false;

	resolve_symbol("glCullFace", ::glCullFace, is_failed);
	resolve_symbol("glFrontFace", ::glFrontFace, is_failed);
	resolve_symbol("glHint", ::glHint, is_failed);
	resolve_symbol("glLineWidth", ::glLineWidth, is_failed);
	resolve_symbol("glPointSize", ::glPointSize, is_failed);
	resolve_symbol("glPolygonMode", ::glPolygonMode, is_failed);
	resolve_symbol("glScissor", ::glScissor, is_failed);
	resolve_symbol("glTexParameterf", ::glTexParameterf, is_failed);
	resolve_symbol("glTexParameterfv", ::glTexParameterfv, is_failed);
	resolve_symbol("glTexParameteri", ::glTexParameteri, is_failed);
	resolve_symbol("glTexParameteriv", ::glTexParameteriv, is_failed);
	resolve_symbol("glTexImage1D", ::glTexImage1D, is_failed);
	resolve_symbol("glTexImage2D", ::glTexImage2D, is_failed);
	resolve_symbol("glDrawBuffer", ::glDrawBuffer, is_failed);
	resolve_symbol("glClear", ::glClear, is_failed);
	resolve_symbol("glClearColor", ::glClearColor, is_failed);
	resolve_symbol("glClearStencil", ::glClearStencil, is_failed);
	resolve_symbol("glClearDepth", ::glClearDepth, is_failed);
	resolve_symbol("glStencilMask", ::glStencilMask, is_failed);
	resolve_symbol("glColorMask", ::glColorMask, is_failed);
	resolve_symbol("glDepthMask", ::glDepthMask, is_failed);
	resolve_symbol("glDisable", ::glDisable, is_failed);
	resolve_symbol("glEnable", ::glEnable, is_failed);
	resolve_symbol("glFinish", ::glFinish, is_failed);
	resolve_symbol("glFlush", ::glFlush, is_failed);
	resolve_symbol("glBlendFunc", ::glBlendFunc, is_failed);
	resolve_symbol("glLogicOp", ::glLogicOp, is_failed);
	resolve_symbol("glStencilFunc", ::glStencilFunc, is_failed);
	resolve_symbol("glStencilOp", ::glStencilOp, is_failed);
	resolve_symbol("glDepthFunc", ::glDepthFunc, is_failed);
	resolve_symbol("glPixelStoref", ::glPixelStoref, is_failed);
	resolve_symbol("glPixelStorei", ::glPixelStorei, is_failed);
	resolve_symbol("glReadBuffer", ::glReadBuffer, is_failed);
	resolve_symbol("glReadPixels", ::glReadPixels, is_failed);
	resolve_symbol("glGetBooleanv", ::glGetBooleanv, is_failed);
	resolve_symbol("glGetDoublev", ::glGetDoublev, is_failed);
	resolve_symbol("glGetError", ::glGetError, is_failed);
	resolve_symbol("glGetFloatv", ::glGetFloatv, is_failed);
	resolve_symbol("glGetIntegerv", ::glGetIntegerv, is_failed);
	resolve_symbol("glGetString", ::glGetString, is_failed);
	resolve_symbol("glGetTexImage", ::glGetTexImage, is_failed);
	resolve_symbol("glGetTexParameterfv", ::glGetTexParameterfv, is_failed);
	resolve_symbol("glGetTexParameteriv", ::glGetTexParameteriv, is_failed);
	resolve_symbol("glGetTexLevelParameterfv", ::glGetTexLevelParameterfv, is_failed);
	resolve_symbol("glGetTexLevelParameteriv", ::glGetTexLevelParameteriv, is_failed);
	resolve_symbol("glIsEnabled", ::glIsEnabled, is_failed);
	resolve_symbol("glDepthRange", ::glDepthRange, is_failed);
	resolve_symbol("glViewport", ::glViewport, is_failed);
	resolve_symbol("glNewList", ::glNewList, is_failed);
	resolve_symbol("glEndList", ::glEndList, is_failed);
	resolve_symbol("glCallList", ::glCallList, is_failed);
	resolve_symbol("glCallLists", ::glCallLists, is_failed);
	resolve_symbol("glDeleteLists", ::glDeleteLists, is_failed);
	resolve_symbol("glGenLists", ::glGenLists, is_failed);
	resolve_symbol("glListBase", ::glListBase, is_failed);
	resolve_symbol("glBegin", ::glBegin, is_failed);
	resolve_symbol("glBitmap", ::glBitmap, is_failed);
	resolve_symbol("glColor3b", ::glColor3b, is_failed);
	resolve_symbol("glColor3bv", ::glColor3bv, is_failed);
	resolve_symbol("glColor3d", ::glColor3d, is_failed);
	resolve_symbol("glColor3dv", ::glColor3dv, is_failed);
	resolve_symbol("glColor3f", ::glColor3f, is_failed);
	resolve_symbol("glColor3fv", ::glColor3fv, is_failed);
	resolve_symbol("glColor3i", ::glColor3i, is_failed);
	resolve_symbol("glColor3iv", ::glColor3iv, is_failed);
	resolve_symbol("glColor3s", ::glColor3s, is_failed);
	resolve_symbol("glColor3sv", ::glColor3sv, is_failed);
	resolve_symbol("glColor3ub", ::glColor3ub, is_failed);
	resolve_symbol("glColor3ubv", ::glColor3ubv, is_failed);
	resolve_symbol("glColor3ui", ::glColor3ui, is_failed);
	resolve_symbol("glColor3uiv", ::glColor3uiv, is_failed);
	resolve_symbol("glColor3us", ::glColor3us, is_failed);
	resolve_symbol("glColor3usv", ::glColor3usv, is_failed);
	resolve_symbol("glColor4b", ::glColor4b, is_failed);
	resolve_symbol("glColor4bv", ::glColor4bv, is_failed);
	resolve_symbol("glColor4d", ::glColor4d, is_failed);
	resolve_symbol("glColor4dv", ::glColor4dv, is_failed);
	resolve_symbol("glColor4f", ::glColor4f, is_failed);
	resolve_symbol("glColor4fv", ::glColor4fv, is_failed);
	resolve_symbol("glColor4i", ::glColor4i, is_failed);
	resolve_symbol("glColor4iv", ::glColor4iv, is_failed);
	resolve_symbol("glColor4s", ::glColor4s, is_failed);
	resolve_symbol("glColor4sv", ::glColor4sv, is_failed);
	resolve_symbol("glColor4ub", ::glColor4ub, is_failed);
	resolve_symbol("glColor4ubv", ::glColor4ubv, is_failed);
	resolve_symbol("glColor4ui", ::glColor4ui, is_failed);
	resolve_symbol("glColor4uiv", ::glColor4uiv, is_failed);
	resolve_symbol("glColor4us", ::glColor4us, is_failed);
	resolve_symbol("glColor4usv", ::glColor4usv, is_failed);
	resolve_symbol("glEdgeFlag", ::glEdgeFlag, is_failed);
	resolve_symbol("glEdgeFlagv", ::glEdgeFlagv, is_failed);
	resolve_symbol("glEnd", ::glEnd, is_failed);
	resolve_symbol("glIndexd", ::glIndexd, is_failed);
	resolve_symbol("glIndexdv", ::glIndexdv, is_failed);
	resolve_symbol("glIndexf", ::glIndexf, is_failed);
	resolve_symbol("glIndexfv", ::glIndexfv, is_failed);
	resolve_symbol("glIndexi", ::glIndexi, is_failed);
	resolve_symbol("glIndexiv", ::glIndexiv, is_failed);
	resolve_symbol("glIndexs", ::glIndexs, is_failed);
	resolve_symbol("glIndexsv", ::glIndexsv, is_failed);
	resolve_symbol("glNormal3b", ::glNormal3b, is_failed);
	resolve_symbol("glNormal3bv", ::glNormal3bv, is_failed);
	resolve_symbol("glNormal3d", ::glNormal3d, is_failed);
	resolve_symbol("glNormal3dv", ::glNormal3dv, is_failed);
	resolve_symbol("glNormal3f", ::glNormal3f, is_failed);
	resolve_symbol("glNormal3fv", ::glNormal3fv, is_failed);
	resolve_symbol("glNormal3i", ::glNormal3i, is_failed);
	resolve_symbol("glNormal3iv", ::glNormal3iv, is_failed);
	resolve_symbol("glNormal3s", ::glNormal3s, is_failed);
	resolve_symbol("glNormal3sv", ::glNormal3sv, is_failed);
	resolve_symbol("glRasterPos2d", ::glRasterPos2d, is_failed);
	resolve_symbol("glRasterPos2dv", ::glRasterPos2dv, is_failed);
	resolve_symbol("glRasterPos2f", ::glRasterPos2f, is_failed);
	resolve_symbol("glRasterPos2fv", ::glRasterPos2fv, is_failed);
	resolve_symbol("glRasterPos2i", ::glRasterPos2i, is_failed);
	resolve_symbol("glRasterPos2iv", ::glRasterPos2iv, is_failed);
	resolve_symbol("glRasterPos2s", ::glRasterPos2s, is_failed);
	resolve_symbol("glRasterPos2sv", ::glRasterPos2sv, is_failed);
	resolve_symbol("glRasterPos3d", ::glRasterPos3d, is_failed);
	resolve_symbol("glRasterPos3dv", ::glRasterPos3dv, is_failed);
	resolve_symbol("glRasterPos3f", ::glRasterPos3f, is_failed);
	resolve_symbol("glRasterPos3fv", ::glRasterPos3fv, is_failed);
	resolve_symbol("glRasterPos3i", ::glRasterPos3i, is_failed);
	resolve_symbol("glRasterPos3iv", ::glRasterPos3iv, is_failed);
	resolve_symbol("glRasterPos3s", ::glRasterPos3s, is_failed);
	resolve_symbol("glRasterPos3sv", ::glRasterPos3sv, is_failed);
	resolve_symbol("glRasterPos4d", ::glRasterPos4d, is_failed);
	resolve_symbol("glRasterPos4dv", ::glRasterPos4dv, is_failed);
	resolve_symbol("glRasterPos4f", ::glRasterPos4f, is_failed);
	resolve_symbol("glRasterPos4fv", ::glRasterPos4fv, is_failed);
	resolve_symbol("glRasterPos4i", ::glRasterPos4i, is_failed);
	resolve_symbol("glRasterPos4iv", ::glRasterPos4iv, is_failed);
	resolve_symbol("glRasterPos4s", ::glRasterPos4s, is_failed);
	resolve_symbol("glRasterPos4sv", ::glRasterPos4sv, is_failed);
	resolve_symbol("glRectd", ::glRectd, is_failed);
	resolve_symbol("glRectdv", ::glRectdv, is_failed);
	resolve_symbol("glRectf", ::glRectf, is_failed);
	resolve_symbol("glRectfv", ::glRectfv, is_failed);
	resolve_symbol("glRecti", ::glRecti, is_failed);
	resolve_symbol("glRectiv", ::glRectiv, is_failed);
	resolve_symbol("glRects", ::glRects, is_failed);
	resolve_symbol("glRectsv", ::glRectsv, is_failed);
	resolve_symbol("glTexCoord1d", ::glTexCoord1d, is_failed);
	resolve_symbol("glTexCoord1dv", ::glTexCoord1dv, is_failed);
	resolve_symbol("glTexCoord1f", ::glTexCoord1f, is_failed);
	resolve_symbol("glTexCoord1fv", ::glTexCoord1fv, is_failed);
	resolve_symbol("glTexCoord1i", ::glTexCoord1i, is_failed);
	resolve_symbol("glTexCoord1iv", ::glTexCoord1iv, is_failed);
	resolve_symbol("glTexCoord1s", ::glTexCoord1s, is_failed);
	resolve_symbol("glTexCoord1sv", ::glTexCoord1sv, is_failed);
	resolve_symbol("glTexCoord2d", ::glTexCoord2d, is_failed);
	resolve_symbol("glTexCoord2dv", ::glTexCoord2dv, is_failed);
	resolve_symbol("glTexCoord2f", ::glTexCoord2f, is_failed);
	resolve_symbol("glTexCoord2fv", ::glTexCoord2fv, is_failed);
	resolve_symbol("glTexCoord2i", ::glTexCoord2i, is_failed);
	resolve_symbol("glTexCoord2iv", ::glTexCoord2iv, is_failed);
	resolve_symbol("glTexCoord2s", ::glTexCoord2s, is_failed);
	resolve_symbol("glTexCoord2sv", ::glTexCoord2sv, is_failed);
	resolve_symbol("glTexCoord3d", ::glTexCoord3d, is_failed);
	resolve_symbol("glTexCoord3dv", ::glTexCoord3dv, is_failed);
	resolve_symbol("glTexCoord3f", ::glTexCoord3f, is_failed);
	resolve_symbol("glTexCoord3fv", ::glTexCoord3fv, is_failed);
	resolve_symbol("glTexCoord3i", ::glTexCoord3i, is_failed);
	resolve_symbol("glTexCoord3iv", ::glTexCoord3iv, is_failed);
	resolve_symbol("glTexCoord3s", ::glTexCoord3s, is_failed);
	resolve_symbol("glTexCoord3sv", ::glTexCoord3sv, is_failed);
	resolve_symbol("glTexCoord4d", ::glTexCoord4d, is_failed);
	resolve_symbol("glTexCoord4dv", ::glTexCoord4dv, is_failed);
	resolve_symbol("glTexCoord4f", ::glTexCoord4f, is_failed);
	resolve_symbol("glTexCoord4fv", ::glTexCoord4fv, is_failed);
	resolve_symbol("glTexCoord4i", ::glTexCoord4i, is_failed);
	resolve_symbol("glTexCoord4iv", ::glTexCoord4iv, is_failed);
	resolve_symbol("glTexCoord4s", ::glTexCoord4s, is_failed);
	resolve_symbol("glTexCoord4sv", ::glTexCoord4sv, is_failed);
	resolve_symbol("glVertex2d", ::glVertex2d, is_failed);
	resolve_symbol("glVertex2dv", ::glVertex2dv, is_failed);
	resolve_symbol("glVertex2f", ::glVertex2f, is_failed);
	resolve_symbol("glVertex2fv", ::glVertex2fv, is_failed);
	resolve_symbol("glVertex2i", ::glVertex2i, is_failed);
	resolve_symbol("glVertex2iv", ::glVertex2iv, is_failed);
	resolve_symbol("glVertex2s", ::glVertex2s, is_failed);
	resolve_symbol("glVertex2sv", ::glVertex2sv, is_failed);
	resolve_symbol("glVertex3d", ::glVertex3d, is_failed);
	resolve_symbol("glVertex3dv", ::glVertex3dv, is_failed);
	resolve_symbol("glVertex3f", ::glVertex3f, is_failed);
	resolve_symbol("glVertex3fv", ::glVertex3fv, is_failed);
	resolve_symbol("glVertex3i", ::glVertex3i, is_failed);
	resolve_symbol("glVertex3iv", ::glVertex3iv, is_failed);
	resolve_symbol("glVertex3s", ::glVertex3s, is_failed);
	resolve_symbol("glVertex3sv", ::glVertex3sv, is_failed);
	resolve_symbol("glVertex4d", ::glVertex4d, is_failed);
	resolve_symbol("glVertex4dv", ::glVertex4dv, is_failed);
	resolve_symbol("glVertex4f", ::glVertex4f, is_failed);
	resolve_symbol("glVertex4fv", ::glVertex4fv, is_failed);
	resolve_symbol("glVertex4i", ::glVertex4i, is_failed);
	resolve_symbol("glVertex4iv", ::glVertex4iv, is_failed);
	resolve_symbol("glVertex4s", ::glVertex4s, is_failed);
	resolve_symbol("glVertex4sv", ::glVertex4sv, is_failed);
	resolve_symbol("glClipPlane", ::glClipPlane, is_failed);
	resolve_symbol("glColorMaterial", ::glColorMaterial, is_failed);
	resolve_symbol("glFogf", ::glFogf, is_failed);
	resolve_symbol("glFogfv", ::glFogfv, is_failed);
	resolve_symbol("glFogi", ::glFogi, is_failed);
	resolve_symbol("glFogiv", ::glFogiv, is_failed);
	resolve_symbol("glLightf", ::glLightf, is_failed);
	resolve_symbol("glLightfv", ::glLightfv, is_failed);
	resolve_symbol("glLighti", ::glLighti, is_failed);
	resolve_symbol("glLightiv", ::glLightiv, is_failed);
	resolve_symbol("glLightModelf", ::glLightModelf, is_failed);
	resolve_symbol("glLightModelfv", ::glLightModelfv, is_failed);
	resolve_symbol("glLightModeli", ::glLightModeli, is_failed);
	resolve_symbol("glLightModeliv", ::glLightModeliv, is_failed);
	resolve_symbol("glLineStipple", ::glLineStipple, is_failed);
	resolve_symbol("glMaterialf", ::glMaterialf, is_failed);
	resolve_symbol("glMaterialfv", ::glMaterialfv, is_failed);
	resolve_symbol("glMateriali", ::glMateriali, is_failed);
	resolve_symbol("glMaterialiv", ::glMaterialiv, is_failed);
	resolve_symbol("glPolygonStipple", ::glPolygonStipple, is_failed);
	resolve_symbol("glShadeModel", ::glShadeModel, is_failed);
	resolve_symbol("glTexEnvf", ::glTexEnvf, is_failed);
	resolve_symbol("glTexEnvfv", ::glTexEnvfv, is_failed);
	resolve_symbol("glTexEnvi", ::glTexEnvi, is_failed);
	resolve_symbol("glTexEnviv", ::glTexEnviv, is_failed);
	resolve_symbol("glTexGend", ::glTexGend, is_failed);
	resolve_symbol("glTexGendv", ::glTexGendv, is_failed);
	resolve_symbol("glTexGenf", ::glTexGenf, is_failed);
	resolve_symbol("glTexGenfv", ::glTexGenfv, is_failed);
	resolve_symbol("glTexGeni", ::glTexGeni, is_failed);
	resolve_symbol("glTexGeniv", ::glTexGeniv, is_failed);
	resolve_symbol("glFeedbackBuffer", ::glFeedbackBuffer, is_failed);
	resolve_symbol("glSelectBuffer", ::glSelectBuffer, is_failed);
	resolve_symbol("glRenderMode", ::glRenderMode, is_failed);
	resolve_symbol("glInitNames", ::glInitNames, is_failed);
	resolve_symbol("glLoadName", ::glLoadName, is_failed);
	resolve_symbol("glPassThrough", ::glPassThrough, is_failed);
	resolve_symbol("glPopName", ::glPopName, is_failed);
	resolve_symbol("glPushName", ::glPushName, is_failed);
	resolve_symbol("glClearAccum", ::glClearAccum, is_failed);
	resolve_symbol("glClearIndex", ::glClearIndex, is_failed);
	resolve_symbol("glIndexMask", ::glIndexMask, is_failed);
	resolve_symbol("glAccum", ::glAccum, is_failed);
	resolve_symbol("glPopAttrib", ::glPopAttrib, is_failed);
	resolve_symbol("glPushAttrib", ::glPushAttrib, is_failed);
	resolve_symbol("glMap1d", ::glMap1d, is_failed);
	resolve_symbol("glMap1f", ::glMap1f, is_failed);
	resolve_symbol("glMap2d", ::glMap2d, is_failed);
	resolve_symbol("glMap2f", ::glMap2f, is_failed);
	resolve_symbol("glMapGrid1d", ::glMapGrid1d, is_failed);
	resolve_symbol("glMapGrid1f", ::glMapGrid1f, is_failed);
	resolve_symbol("glMapGrid2d", ::glMapGrid2d, is_failed);
	resolve_symbol("glMapGrid2f", ::glMapGrid2f, is_failed);
	resolve_symbol("glEvalCoord1d", ::glEvalCoord1d, is_failed);
	resolve_symbol("glEvalCoord1dv", ::glEvalCoord1dv, is_failed);
	resolve_symbol("glEvalCoord1f", ::glEvalCoord1f, is_failed);
	resolve_symbol("glEvalCoord1fv", ::glEvalCoord1fv, is_failed);
	resolve_symbol("glEvalCoord2d", ::glEvalCoord2d, is_failed);
	resolve_symbol("glEvalCoord2dv", ::glEvalCoord2dv, is_failed);
	resolve_symbol("glEvalCoord2f", ::glEvalCoord2f, is_failed);
	resolve_symbol("glEvalCoord2fv", ::glEvalCoord2fv, is_failed);
	resolve_symbol("glEvalMesh1", ::glEvalMesh1, is_failed);
	resolve_symbol("glEvalPoint1", ::glEvalPoint1, is_failed);
	resolve_symbol("glEvalMesh2", ::glEvalMesh2, is_failed);
	resolve_symbol("glEvalPoint2", ::glEvalPoint2, is_failed);
	resolve_symbol("glAlphaFunc", ::glAlphaFunc, is_failed);
	resolve_symbol("glPixelZoom", ::glPixelZoom, is_failed);
	resolve_symbol("glPixelTransferf", ::glPixelTransferf, is_failed);
	resolve_symbol("glPixelTransferi", ::glPixelTransferi, is_failed);
	resolve_symbol("glPixelMapfv", ::glPixelMapfv, is_failed);
	resolve_symbol("glPixelMapuiv", ::glPixelMapuiv, is_failed);
	resolve_symbol("glPixelMapusv", ::glPixelMapusv, is_failed);
	resolve_symbol("glCopyPixels", ::glCopyPixels, is_failed);
	resolve_symbol("glDrawPixels", ::glDrawPixels, is_failed);
	resolve_symbol("glGetClipPlane", ::glGetClipPlane, is_failed);
	resolve_symbol("glGetLightfv", ::glGetLightfv, is_failed);
	resolve_symbol("glGetLightiv", ::glGetLightiv, is_failed);
	resolve_symbol("glGetMapdv", ::glGetMapdv, is_failed);
	resolve_symbol("glGetMapfv", ::glGetMapfv, is_failed);
	resolve_symbol("glGetMapiv", ::glGetMapiv, is_failed);
	resolve_symbol("glGetMaterialfv", ::glGetMaterialfv, is_failed);
	resolve_symbol("glGetMaterialiv", ::glGetMaterialiv, is_failed);
	resolve_symbol("glGetPixelMapfv", ::glGetPixelMapfv, is_failed);
	resolve_symbol("glGetPixelMapuiv", ::glGetPixelMapuiv, is_failed);
	resolve_symbol("glGetPixelMapusv", ::glGetPixelMapusv, is_failed);
	resolve_symbol("glGetPolygonStipple", ::glGetPolygonStipple, is_failed);
	resolve_symbol("glGetTexEnvfv", ::glGetTexEnvfv, is_failed);
	resolve_symbol("glGetTexEnviv", ::glGetTexEnviv, is_failed);
	resolve_symbol("glGetTexGendv", ::glGetTexGendv, is_failed);
	resolve_symbol("glGetTexGenfv", ::glGetTexGenfv, is_failed);
	resolve_symbol("glGetTexGeniv", ::glGetTexGeniv, is_failed);
	resolve_symbol("glIsList", ::glIsList, is_failed);
	resolve_symbol("glFrustum", ::glFrustum, is_failed);
	resolve_symbol("glLoadIdentity", ::glLoadIdentity, is_failed);
	resolve_symbol("glLoadMatrixf", ::glLoadMatrixf, is_failed);
	resolve_symbol("glLoadMatrixd", ::glLoadMatrixd, is_failed);
	resolve_symbol("glMatrixMode", ::glMatrixMode, is_failed);
	resolve_symbol("glMultMatrixf", ::glMultMatrixf, is_failed);
	resolve_symbol("glMultMatrixd", ::glMultMatrixd, is_failed);
	resolve_symbol("glOrtho", ::glOrtho, is_failed);
	resolve_symbol("glPopMatrix", ::glPopMatrix, is_failed);
	resolve_symbol("glPushMatrix", ::glPushMatrix, is_failed);
	resolve_symbol("glRotated", ::glRotated, is_failed);
	resolve_symbol("glRotatef", ::glRotatef, is_failed);
	resolve_symbol("glScaled", ::glScaled, is_failed);
	resolve_symbol("glScalef", ::glScalef, is_failed);
	resolve_symbol("glTranslated", ::glTranslated, is_failed);
	resolve_symbol("glTranslatef", ::glTranslatef, is_failed);

	return !is_failed;
}

bool OglExtensionManagerImpl::resolve_v1_1()
{
	auto is_failed = false;

	resolve_symbol("glDrawArrays", ::glDrawArrays, is_failed);
	resolve_symbol("glDrawElements", ::glDrawElements, is_failed);
	resolve_symbol("glGetPointerv", ::glGetPointerv, is_failed);
	resolve_symbol("glPolygonOffset", ::glPolygonOffset, is_failed);
	resolve_symbol("glCopyTexImage1D", ::glCopyTexImage1D, is_failed);
	resolve_symbol("glCopyTexImage2D", ::glCopyTexImage2D, is_failed);
	resolve_symbol("glCopyTexSubImage1D", ::glCopyTexSubImage1D, is_failed);
	resolve_symbol("glCopyTexSubImage2D", ::glCopyTexSubImage2D, is_failed);
	resolve_symbol("glTexSubImage1D", ::glTexSubImage1D, is_failed);
	resolve_symbol("glTexSubImage2D", ::glTexSubImage2D, is_failed);
	resolve_symbol("glBindTexture", ::glBindTexture, is_failed);
	resolve_symbol("glDeleteTextures", ::glDeleteTextures, is_failed);
	resolve_symbol("glGenTextures", ::glGenTextures, is_failed);
	resolve_symbol("glIsTexture", ::glIsTexture, is_failed);
	resolve_symbol("glArrayElement", ::glArrayElement, is_failed);
	resolve_symbol("glColorPointer", ::glColorPointer, is_failed);
	resolve_symbol("glDisableClientState", ::glDisableClientState, is_failed);
	resolve_symbol("glEdgeFlagPointer", ::glEdgeFlagPointer, is_failed);
	resolve_symbol("glEnableClientState", ::glEnableClientState, is_failed);
	resolve_symbol("glIndexPointer", ::glIndexPointer, is_failed);
	resolve_symbol("glInterleavedArrays", ::glInterleavedArrays, is_failed);
	resolve_symbol("glNormalPointer", ::glNormalPointer, is_failed);
	resolve_symbol("glTexCoordPointer", ::glTexCoordPointer, is_failed);
	resolve_symbol("glVertexPointer", ::glVertexPointer, is_failed);
	resolve_symbol("glAreTexturesResident", ::glAreTexturesResident, is_failed);
	resolve_symbol("glPrioritizeTextures", ::glPrioritizeTextures, is_failed);
	resolve_symbol("glIndexub", ::glIndexub, is_failed);
	resolve_symbol("glIndexubv", ::glIndexubv, is_failed);
	resolve_symbol("glPopClientAttrib", ::glPopClientAttrib, is_failed);
	resolve_symbol("glPushClientAttrib", ::glPushClientAttrib, is_failed);

	return !is_failed;
}

bool OglExtensionManagerImpl::resolve_v1_2()
{
	auto is_failed = false;

	resolve_symbol("glDrawRangeElements", ::glDrawRangeElements, is_failed);
	resolve_symbol("glTexImage3D", ::glTexImage3D, is_failed);
	resolve_symbol("glTexSubImage3D", ::glTexSubImage3D, is_failed);
	resolve_symbol("glCopyTexSubImage3D", ::glCopyTexSubImage3D, is_failed);

	return !is_failed;
}

bool OglExtensionManagerImpl::resolve_v1_3()
{
	auto is_failed = false;

	resolve_symbol("glActiveTexture", ::glActiveTexture, is_failed);
	resolve_symbol("glSampleCoverage", ::glSampleCoverage, is_failed);
	resolve_symbol("glCompressedTexImage3D", ::glCompressedTexImage3D, is_failed);
	resolve_symbol("glCompressedTexImage2D", ::glCompressedTexImage2D, is_failed);
	resolve_symbol("glCompressedTexImage1D", ::glCompressedTexImage1D, is_failed);
	resolve_symbol("glCompressedTexSubImage3D", ::glCompressedTexSubImage3D, is_failed);
	resolve_symbol("glCompressedTexSubImage2D", ::glCompressedTexSubImage2D, is_failed);
	resolve_symbol("glCompressedTexSubImage1D", ::glCompressedTexSubImage1D, is_failed);
	resolve_symbol("glGetCompressedTexImage", ::glGetCompressedTexImage, is_failed);
	resolve_symbol("glClientActiveTexture", ::glClientActiveTexture, is_failed);
	resolve_symbol("glMultiTexCoord1d", ::glMultiTexCoord1d, is_failed);
	resolve_symbol("glMultiTexCoord1dv", ::glMultiTexCoord1dv, is_failed);
	resolve_symbol("glMultiTexCoord1f", ::glMultiTexCoord1f, is_failed);
	resolve_symbol("glMultiTexCoord1fv", ::glMultiTexCoord1fv, is_failed);
	resolve_symbol("glMultiTexCoord1i", ::glMultiTexCoord1i, is_failed);
	resolve_symbol("glMultiTexCoord1iv", ::glMultiTexCoord1iv, is_failed);
	resolve_symbol("glMultiTexCoord1s", ::glMultiTexCoord1s, is_failed);
	resolve_symbol("glMultiTexCoord1sv", ::glMultiTexCoord1sv, is_failed);
	resolve_symbol("glMultiTexCoord2d", ::glMultiTexCoord2d, is_failed);
	resolve_symbol("glMultiTexCoord2dv", ::glMultiTexCoord2dv, is_failed);
	resolve_symbol("glMultiTexCoord2f", ::glMultiTexCoord2f, is_failed);
	resolve_symbol("glMultiTexCoord2fv", ::glMultiTexCoord2fv, is_failed);
	resolve_symbol("glMultiTexCoord2i", ::glMultiTexCoord2i, is_failed);
	resolve_symbol("glMultiTexCoord2iv", ::glMultiTexCoord2iv, is_failed);
	resolve_symbol("glMultiTexCoord2s", ::glMultiTexCoord2s, is_failed);
	resolve_symbol("glMultiTexCoord2sv", ::glMultiTexCoord2sv, is_failed);
	resolve_symbol("glMultiTexCoord3d", ::glMultiTexCoord3d, is_failed);
	resolve_symbol("glMultiTexCoord3dv", ::glMultiTexCoord3dv, is_failed);
	resolve_symbol("glMultiTexCoord3f", ::glMultiTexCoord3f, is_failed);
	resolve_symbol("glMultiTexCoord3fv", ::glMultiTexCoord3fv, is_failed);
	resolve_symbol("glMultiTexCoord3i", ::glMultiTexCoord3i, is_failed);
	resolve_symbol("glMultiTexCoord3iv", ::glMultiTexCoord3iv, is_failed);
	resolve_symbol("glMultiTexCoord3s", ::glMultiTexCoord3s, is_failed);
	resolve_symbol("glMultiTexCoord3sv", ::glMultiTexCoord3sv, is_failed);
	resolve_symbol("glMultiTexCoord4d", ::glMultiTexCoord4d, is_failed);
	resolve_symbol("glMultiTexCoord4dv", ::glMultiTexCoord4dv, is_failed);
	resolve_symbol("glMultiTexCoord4f", ::glMultiTexCoord4f, is_failed);
	resolve_symbol("glMultiTexCoord4fv", ::glMultiTexCoord4fv, is_failed);
	resolve_symbol("glMultiTexCoord4i", ::glMultiTexCoord4i, is_failed);
	resolve_symbol("glMultiTexCoord4iv", ::glMultiTexCoord4iv, is_failed);
	resolve_symbol("glMultiTexCoord4s", ::glMultiTexCoord4s, is_failed);
	resolve_symbol("glMultiTexCoord4sv", ::glMultiTexCoord4sv, is_failed);
	resolve_symbol("glLoadTransposeMatrixf", ::glLoadTransposeMatrixf, is_failed);
	resolve_symbol("glLoadTransposeMatrixd", ::glLoadTransposeMatrixd, is_failed);
	resolve_symbol("glMultTransposeMatrixf", ::glMultTransposeMatrixf, is_failed);
	resolve_symbol("glMultTransposeMatrixd", ::glMultTransposeMatrixd, is_failed);

	return !is_failed;
}

bool OglExtensionManagerImpl::resolve_v1_4()
{
	auto is_failed = false;

	resolve_symbol("glBlendFuncSeparate", ::glBlendFuncSeparate, is_failed);
	resolve_symbol("glMultiDrawArrays", ::glMultiDrawArrays, is_failed);
	resolve_symbol("glMultiDrawElements", ::glMultiDrawElements, is_failed);
	resolve_symbol("glPointParameterf", ::glPointParameterf, is_failed);
	resolve_symbol("glPointParameterfv", ::glPointParameterfv, is_failed);
	resolve_symbol("glPointParameteri", ::glPointParameteri, is_failed);
	resolve_symbol("glPointParameteriv", ::glPointParameteriv, is_failed);
	resolve_symbol("glFogCoordf", ::glFogCoordf, is_failed);
	resolve_symbol("glFogCoordfv", ::glFogCoordfv, is_failed);
	resolve_symbol("glFogCoordd", ::glFogCoordd, is_failed);
	resolve_symbol("glFogCoorddv", ::glFogCoorddv, is_failed);
	resolve_symbol("glFogCoordPointer", ::glFogCoordPointer, is_failed);
	resolve_symbol("glSecondaryColor3b", ::glSecondaryColor3b, is_failed);
	resolve_symbol("glSecondaryColor3bv", ::glSecondaryColor3bv, is_failed);
	resolve_symbol("glSecondaryColor3d", ::glSecondaryColor3d, is_failed);
	resolve_symbol("glSecondaryColor3dv", ::glSecondaryColor3dv, is_failed);
	resolve_symbol("glSecondaryColor3f", ::glSecondaryColor3f, is_failed);
	resolve_symbol("glSecondaryColor3fv", ::glSecondaryColor3fv, is_failed);
	resolve_symbol("glSecondaryColor3i", ::glSecondaryColor3i, is_failed);
	resolve_symbol("glSecondaryColor3iv", ::glSecondaryColor3iv, is_failed);
	resolve_symbol("glSecondaryColor3s", ::glSecondaryColor3s, is_failed);
	resolve_symbol("glSecondaryColor3sv", ::glSecondaryColor3sv, is_failed);
	resolve_symbol("glSecondaryColor3ub", ::glSecondaryColor3ub, is_failed);
	resolve_symbol("glSecondaryColor3ubv", ::glSecondaryColor3ubv, is_failed);
	resolve_symbol("glSecondaryColor3ui", ::glSecondaryColor3ui, is_failed);
	resolve_symbol("glSecondaryColor3uiv", ::glSecondaryColor3uiv, is_failed);
	resolve_symbol("glSecondaryColor3us", ::glSecondaryColor3us, is_failed);
	resolve_symbol("glSecondaryColor3usv", ::glSecondaryColor3usv, is_failed);
	resolve_symbol("glSecondaryColorPointer", ::glSecondaryColorPointer, is_failed);
	resolve_symbol("glWindowPos2d", ::glWindowPos2d, is_failed);
	resolve_symbol("glWindowPos2dv", ::glWindowPos2dv, is_failed);
	resolve_symbol("glWindowPos2f", ::glWindowPos2f, is_failed);
	resolve_symbol("glWindowPos2fv", ::glWindowPos2fv, is_failed);
	resolve_symbol("glWindowPos2i", ::glWindowPos2i, is_failed);
	resolve_symbol("glWindowPos2iv", ::glWindowPos2iv, is_failed);
	resolve_symbol("glWindowPos2s", ::glWindowPos2s, is_failed);
	resolve_symbol("glWindowPos2sv", ::glWindowPos2sv, is_failed);
	resolve_symbol("glWindowPos3d", ::glWindowPos3d, is_failed);
	resolve_symbol("glWindowPos3dv", ::glWindowPos3dv, is_failed);
	resolve_symbol("glWindowPos3f", ::glWindowPos3f, is_failed);
	resolve_symbol("glWindowPos3fv", ::glWindowPos3fv, is_failed);
	resolve_symbol("glWindowPos3i", ::glWindowPos3i, is_failed);
	resolve_symbol("glWindowPos3iv", ::glWindowPos3iv, is_failed);
	resolve_symbol("glWindowPos3s", ::glWindowPos3s, is_failed);
	resolve_symbol("glWindowPos3sv", ::glWindowPos3sv, is_failed);
	resolve_symbol("glBlendColor", ::glBlendColor, is_failed);
	resolve_symbol("glBlendEquation", ::glBlendEquation, is_failed);

	return !is_failed;
}

bool OglExtensionManagerImpl::resolve_v1_5()
{
	auto is_failed = false;

	resolve_symbol("glGenQueries", ::glGenQueries, is_failed);
	resolve_symbol("glDeleteQueries", ::glDeleteQueries, is_failed);
	resolve_symbol("glIsQuery", ::glIsQuery, is_failed);
	resolve_symbol("glBeginQuery", ::glBeginQuery, is_failed);
	resolve_symbol("glEndQuery", ::glEndQuery, is_failed);
	resolve_symbol("glGetQueryiv", ::glGetQueryiv, is_failed);
	resolve_symbol("glGetQueryObjectiv", ::glGetQueryObjectiv, is_failed);
	resolve_symbol("glGetQueryObjectuiv", ::glGetQueryObjectuiv, is_failed);
	resolve_symbol("glBindBuffer", ::glBindBuffer, is_failed);
	resolve_symbol("glDeleteBuffers", ::glDeleteBuffers, is_failed);
	resolve_symbol("glGenBuffers", ::glGenBuffers, is_failed);
	resolve_symbol("glIsBuffer", ::glIsBuffer, is_failed);
	resolve_symbol("glBufferData", ::glBufferData, is_failed);
	resolve_symbol("glBufferSubData", ::glBufferSubData, is_failed);
	resolve_symbol("glGetBufferSubData", ::glGetBufferSubData, is_failed);
	resolve_symbol("glMapBuffer", ::glMapBuffer, is_failed);
	resolve_symbol("glUnmapBuffer", ::glUnmapBuffer, is_failed);
	resolve_symbol("glGetBufferParameteriv", ::glGetBufferParameteriv, is_failed);
	resolve_symbol("glGetBufferPointerv", ::glGetBufferPointerv, is_failed);

	return !is_failed;
}

bool OglExtensionManagerImpl::resolve_v2_0()
{
	auto is_failed = false;

	resolve_symbol("glBlendEquationSeparate", ::glBlendEquationSeparate, is_failed);
	resolve_symbol("glDrawBuffers", ::glDrawBuffers, is_failed);
	resolve_symbol("glStencilOpSeparate", ::glStencilOpSeparate, is_failed);
	resolve_symbol("glStencilFuncSeparate", ::glStencilFuncSeparate, is_failed);
	resolve_symbol("glStencilMaskSeparate", ::glStencilMaskSeparate, is_failed);
	resolve_symbol("glAttachShader", ::glAttachShader, is_failed);
	resolve_symbol("glBindAttribLocation", ::glBindAttribLocation, is_failed);
	resolve_symbol("glCompileShader", ::glCompileShader, is_failed);
	resolve_symbol("glCreateProgram", ::glCreateProgram, is_failed);
	resolve_symbol("glCreateShader", ::glCreateShader, is_failed);
	resolve_symbol("glDeleteProgram", ::glDeleteProgram, is_failed);
	resolve_symbol("glDeleteShader", ::glDeleteShader, is_failed);
	resolve_symbol("glDetachShader", ::glDetachShader, is_failed);
	resolve_symbol("glDisableVertexAttribArray", ::glDisableVertexAttribArray, is_failed);
	resolve_symbol("glEnableVertexAttribArray", ::glEnableVertexAttribArray, is_failed);
	resolve_symbol("glGetActiveAttrib", ::glGetActiveAttrib, is_failed);
	resolve_symbol("glGetActiveUniform", ::glGetActiveUniform, is_failed);
	resolve_symbol("glGetAttachedShaders", ::glGetAttachedShaders, is_failed);
	resolve_symbol("glGetAttribLocation", ::glGetAttribLocation, is_failed);
	resolve_symbol("glGetProgramiv", ::glGetProgramiv, is_failed);
	resolve_symbol("glGetProgramInfoLog", ::glGetProgramInfoLog, is_failed);
	resolve_symbol("glGetShaderiv", ::glGetShaderiv, is_failed);
	resolve_symbol("glGetShaderInfoLog", ::glGetShaderInfoLog, is_failed);
	resolve_symbol("glGetShaderSource", ::glGetShaderSource, is_failed);
	resolve_symbol("glGetUniformLocation", ::glGetUniformLocation, is_failed);
	resolve_symbol("glGetUniformfv", ::glGetUniformfv, is_failed);
	resolve_symbol("glGetUniformiv", ::glGetUniformiv, is_failed);
	resolve_symbol("glGetVertexAttribdv", ::glGetVertexAttribdv, is_failed);
	resolve_symbol("glGetVertexAttribfv", ::glGetVertexAttribfv, is_failed);
	resolve_symbol("glGetVertexAttribiv", ::glGetVertexAttribiv, is_failed);
	resolve_symbol("glGetVertexAttribPointerv", ::glGetVertexAttribPointerv, is_failed);
	resolve_symbol("glIsProgram", ::glIsProgram, is_failed);
	resolve_symbol("glIsShader", ::glIsShader, is_failed);
	resolve_symbol("glLinkProgram", ::glLinkProgram, is_failed);
	resolve_symbol("glShaderSource", ::glShaderSource, is_failed);
	resolve_symbol("glUseProgram", ::glUseProgram, is_failed);
	resolve_symbol("glUniform1f", ::glUniform1f, is_failed);
	resolve_symbol("glUniform2f", ::glUniform2f, is_failed);
	resolve_symbol("glUniform3f", ::glUniform3f, is_failed);
	resolve_symbol("glUniform4f", ::glUniform4f, is_failed);
	resolve_symbol("glUniform1i", ::glUniform1i, is_failed);
	resolve_symbol("glUniform2i", ::glUniform2i, is_failed);
	resolve_symbol("glUniform3i", ::glUniform3i, is_failed);
	resolve_symbol("glUniform4i", ::glUniform4i, is_failed);
	resolve_symbol("glUniform1fv", ::glUniform1fv, is_failed);
	resolve_symbol("glUniform2fv", ::glUniform2fv, is_failed);
	resolve_symbol("glUniform3fv", ::glUniform3fv, is_failed);
	resolve_symbol("glUniform4fv", ::glUniform4fv, is_failed);
	resolve_symbol("glUniform1iv", ::glUniform1iv, is_failed);
	resolve_symbol("glUniform2iv", ::glUniform2iv, is_failed);
	resolve_symbol("glUniform3iv", ::glUniform3iv, is_failed);
	resolve_symbol("glUniform4iv", ::glUniform4iv, is_failed);
	resolve_symbol("glUniformMatrix2fv", ::glUniformMatrix2fv, is_failed);
	resolve_symbol("glUniformMatrix3fv", ::glUniformMatrix3fv, is_failed);
	resolve_symbol("glUniformMatrix4fv", ::glUniformMatrix4fv, is_failed);
	resolve_symbol("glValidateProgram", ::glValidateProgram, is_failed);
	resolve_symbol("glVertexAttrib1d", ::glVertexAttrib1d, is_failed);
	resolve_symbol("glVertexAttrib1dv", ::glVertexAttrib1dv, is_failed);
	resolve_symbol("glVertexAttrib1f", ::glVertexAttrib1f, is_failed);
	resolve_symbol("glVertexAttrib1fv", ::glVertexAttrib1fv, is_failed);
	resolve_symbol("glVertexAttrib1s", ::glVertexAttrib1s, is_failed);
	resolve_symbol("glVertexAttrib1sv", ::glVertexAttrib1sv, is_failed);
	resolve_symbol("glVertexAttrib2d", ::glVertexAttrib2d, is_failed);
	resolve_symbol("glVertexAttrib2dv", ::glVertexAttrib2dv, is_failed);
	resolve_symbol("glVertexAttrib2f", ::glVertexAttrib2f, is_failed);
	resolve_symbol("glVertexAttrib2fv", ::glVertexAttrib2fv, is_failed);
	resolve_symbol("glVertexAttrib2s", ::glVertexAttrib2s, is_failed);
	resolve_symbol("glVertexAttrib2sv", ::glVertexAttrib2sv, is_failed);
	resolve_symbol("glVertexAttrib3d", ::glVertexAttrib3d, is_failed);
	resolve_symbol("glVertexAttrib3dv", ::glVertexAttrib3dv, is_failed);
	resolve_symbol("glVertexAttrib3f", ::glVertexAttrib3f, is_failed);
	resolve_symbol("glVertexAttrib3fv", ::glVertexAttrib3fv, is_failed);
	resolve_symbol("glVertexAttrib3s", ::glVertexAttrib3s, is_failed);
	resolve_symbol("glVertexAttrib3sv", ::glVertexAttrib3sv, is_failed);
	resolve_symbol("glVertexAttrib4Nbv", ::glVertexAttrib4Nbv, is_failed);
	resolve_symbol("glVertexAttrib4Niv", ::glVertexAttrib4Niv, is_failed);
	resolve_symbol("glVertexAttrib4Nsv", ::glVertexAttrib4Nsv, is_failed);
	resolve_symbol("glVertexAttrib4Nub", ::glVertexAttrib4Nub, is_failed);
	resolve_symbol("glVertexAttrib4Nubv", ::glVertexAttrib4Nubv, is_failed);
	resolve_symbol("glVertexAttrib4Nuiv", ::glVertexAttrib4Nuiv, is_failed);
	resolve_symbol("glVertexAttrib4Nusv", ::glVertexAttrib4Nusv, is_failed);
	resolve_symbol("glVertexAttrib4bv", ::glVertexAttrib4bv, is_failed);
	resolve_symbol("glVertexAttrib4d", ::glVertexAttrib4d, is_failed);
	resolve_symbol("glVertexAttrib4dv", ::glVertexAttrib4dv, is_failed);
	resolve_symbol("glVertexAttrib4f", ::glVertexAttrib4f, is_failed);
	resolve_symbol("glVertexAttrib4fv", ::glVertexAttrib4fv, is_failed);
	resolve_symbol("glVertexAttrib4iv", ::glVertexAttrib4iv, is_failed);
	resolve_symbol("glVertexAttrib4s", ::glVertexAttrib4s, is_failed);
	resolve_symbol("glVertexAttrib4sv", ::glVertexAttrib4sv, is_failed);
	resolve_symbol("glVertexAttrib4ubv", ::glVertexAttrib4ubv, is_failed);
	resolve_symbol("glVertexAttrib4uiv", ::glVertexAttrib4uiv, is_failed);
	resolve_symbol("glVertexAttrib4usv", ::glVertexAttrib4usv, is_failed);
	resolve_symbol("glVertexAttribPointer", ::glVertexAttribPointer, is_failed);

	return !is_failed;
}

bool OglExtensionManagerImpl::resolve_v2_1()
{
	auto is_failed = false;

	resolve_symbol("glUniformMatrix2x3fv", ::glUniformMatrix2x3fv, is_failed);
	resolve_symbol("glUniformMatrix3x2fv", ::glUniformMatrix3x2fv, is_failed);
	resolve_symbol("glUniformMatrix2x4fv", ::glUniformMatrix2x4fv, is_failed);
	resolve_symbol("glUniformMatrix4x2fv", ::glUniformMatrix4x2fv, is_failed);
	resolve_symbol("glUniformMatrix3x4fv", ::glUniformMatrix3x4fv, is_failed);
	resolve_symbol("glUniformMatrix4x3fv", ::glUniformMatrix4x3fv, is_failed);

	return is_failed;
}

bool OglExtensionManagerImpl::resolve_v3_0()
{
	auto is_failed = false;

	resolve_symbol("glColorMaski", ::glColorMaski, is_failed);
	resolve_symbol("glGetBooleani_v", ::glGetBooleani_v, is_failed);
	resolve_symbol("glGetIntegeri_v", ::glGetIntegeri_v, is_failed);
	resolve_symbol("glEnablei", ::glEnablei, is_failed);
	resolve_symbol("glDisablei", ::glDisablei, is_failed);
	resolve_symbol("glIsEnabledi", ::glIsEnabledi, is_failed);
	resolve_symbol("glBeginTransformFeedback", ::glBeginTransformFeedback, is_failed);
	resolve_symbol("glEndTransformFeedback", ::glEndTransformFeedback, is_failed);
	resolve_symbol("glBindBufferRange", ::glBindBufferRange, is_failed);
	resolve_symbol("glBindBufferBase", ::glBindBufferBase, is_failed);
	resolve_symbol("glTransformFeedbackVaryings", ::glTransformFeedbackVaryings, is_failed);
	resolve_symbol("glGetTransformFeedbackVarying", ::glGetTransformFeedbackVarying, is_failed);
	resolve_symbol("glClampColor", ::glClampColor, is_failed);
	resolve_symbol("glBeginConditionalRender", ::glBeginConditionalRender, is_failed);
	resolve_symbol("glEndConditionalRender", ::glEndConditionalRender, is_failed);
	resolve_symbol("glVertexAttribIPointer", ::glVertexAttribIPointer, is_failed);
	resolve_symbol("glGetVertexAttribIiv", ::glGetVertexAttribIiv, is_failed);
	resolve_symbol("glGetVertexAttribIuiv", ::glGetVertexAttribIuiv, is_failed);
	resolve_symbol("glVertexAttribI1i", ::glVertexAttribI1i, is_failed);
	resolve_symbol("glVertexAttribI2i", ::glVertexAttribI2i, is_failed);
	resolve_symbol("glVertexAttribI3i", ::glVertexAttribI3i, is_failed);
	resolve_symbol("glVertexAttribI4i", ::glVertexAttribI4i, is_failed);
	resolve_symbol("glVertexAttribI1ui", ::glVertexAttribI1ui, is_failed);
	resolve_symbol("glVertexAttribI2ui", ::glVertexAttribI2ui, is_failed);
	resolve_symbol("glVertexAttribI3ui", ::glVertexAttribI3ui, is_failed);
	resolve_symbol("glVertexAttribI4ui", ::glVertexAttribI4ui, is_failed);
	resolve_symbol("glVertexAttribI1iv", ::glVertexAttribI1iv, is_failed);
	resolve_symbol("glVertexAttribI2iv", ::glVertexAttribI2iv, is_failed);
	resolve_symbol("glVertexAttribI3iv", ::glVertexAttribI3iv, is_failed);
	resolve_symbol("glVertexAttribI4iv", ::glVertexAttribI4iv, is_failed);
	resolve_symbol("glVertexAttribI1uiv", ::glVertexAttribI1uiv, is_failed);
	resolve_symbol("glVertexAttribI2uiv", ::glVertexAttribI2uiv, is_failed);
	resolve_symbol("glVertexAttribI3uiv", ::glVertexAttribI3uiv, is_failed);
	resolve_symbol("glVertexAttribI4uiv", ::glVertexAttribI4uiv, is_failed);
	resolve_symbol("glVertexAttribI4bv", ::glVertexAttribI4bv, is_failed);
	resolve_symbol("glVertexAttribI4sv", ::glVertexAttribI4sv, is_failed);
	resolve_symbol("glVertexAttribI4ubv", ::glVertexAttribI4ubv, is_failed);
	resolve_symbol("glVertexAttribI4usv", ::glVertexAttribI4usv, is_failed);
	resolve_symbol("glGetUniformuiv", ::glGetUniformuiv, is_failed);
	resolve_symbol("glBindFragDataLocation", ::glBindFragDataLocation, is_failed);
	resolve_symbol("glGetFragDataLocation", ::glGetFragDataLocation, is_failed);
	resolve_symbol("glUniform1ui", ::glUniform1ui, is_failed);
	resolve_symbol("glUniform2ui", ::glUniform2ui, is_failed);
	resolve_symbol("glUniform3ui", ::glUniform3ui, is_failed);
	resolve_symbol("glUniform4ui", ::glUniform4ui, is_failed);
	resolve_symbol("glUniform1uiv", ::glUniform1uiv, is_failed);
	resolve_symbol("glUniform2uiv", ::glUniform2uiv, is_failed);
	resolve_symbol("glUniform3uiv", ::glUniform3uiv, is_failed);
	resolve_symbol("glUniform4uiv", ::glUniform4uiv, is_failed);
	resolve_symbol("glTexParameterIiv", ::glTexParameterIiv, is_failed);
	resolve_symbol("glTexParameterIuiv", ::glTexParameterIuiv, is_failed);
	resolve_symbol("glGetTexParameterIiv", ::glGetTexParameterIiv, is_failed);
	resolve_symbol("glGetTexParameterIuiv", ::glGetTexParameterIuiv, is_failed);
	resolve_symbol("glClearBufferiv", ::glClearBufferiv, is_failed);
	resolve_symbol("glClearBufferuiv", ::glClearBufferuiv, is_failed);
	resolve_symbol("glClearBufferfv", ::glClearBufferfv, is_failed);
	resolve_symbol("glClearBufferfi", ::glClearBufferfi, is_failed);
	resolve_symbol("glGetStringi", ::glGetStringi, is_failed);

	return !is_failed;
}

bool OglExtensionManagerImpl::resolve_v3_1()
{
	auto is_failed = false;

	resolve_symbol("glDrawArraysInstanced", ::glDrawArraysInstanced, is_failed);
	resolve_symbol("glDrawElementsInstanced", ::glDrawElementsInstanced, is_failed);
	resolve_symbol("glTexBuffer", ::glTexBuffer, is_failed);
	resolve_symbol("glPrimitiveRestartIndex", ::glPrimitiveRestartIndex, is_failed);

	return is_failed;
}

bool OglExtensionManagerImpl::resolve_arb_color_buffer_float()
{
	auto is_failed = false;

	resolve_symbol("glClampColorARB", ::glClampColorARB, is_failed);

	return !is_failed;
}

bool OglExtensionManagerImpl::resolve_arb_copy_buffer()
{
	auto is_failed = false;

	resolve_symbol("glCopyBufferSubData", ::glCopyBufferSubData, is_failed);

	return !is_failed;
}

bool OglExtensionManagerImpl::resolve_arb_framebuffer_object()
{
	auto is_failed = false;

	resolve_symbol("glIsRenderbuffer", ::glIsRenderbuffer, is_failed);
	resolve_symbol("glBindRenderbuffer", ::glBindRenderbuffer, is_failed);
	resolve_symbol("glDeleteRenderbuffers", ::glDeleteRenderbuffers, is_failed);
	resolve_symbol("glGenRenderbuffers", ::glGenRenderbuffers, is_failed);
	resolve_symbol("glRenderbufferStorage", ::glRenderbufferStorage, is_failed);
	resolve_symbol("glGetRenderbufferParameteriv", ::glGetRenderbufferParameteriv, is_failed);
	resolve_symbol("glIsFramebuffer", ::glIsFramebuffer, is_failed);
	resolve_symbol("glBindFramebuffer", ::glBindFramebuffer, is_failed);
	resolve_symbol("glDeleteFramebuffers", ::glDeleteFramebuffers, is_failed);
	resolve_symbol("glGenFramebuffers", ::glGenFramebuffers, is_failed);
	resolve_symbol("glCheckFramebufferStatus", ::glCheckFramebufferStatus, is_failed);
	resolve_symbol("glFramebufferTexture1D", ::glFramebufferTexture1D, is_failed);
	resolve_symbol("glFramebufferTexture2D", ::glFramebufferTexture2D, is_failed);
	resolve_symbol("glFramebufferTexture3D", ::glFramebufferTexture3D, is_failed);
	resolve_symbol("glFramebufferRenderbuffer", ::glFramebufferRenderbuffer, is_failed);
	resolve_symbol("glGetFramebufferAttachmentParameteriv", ::glGetFramebufferAttachmentParameteriv, is_failed);
	resolve_symbol("glGenerateMipmap", ::glGenerateMipmap, is_failed);
	resolve_symbol("glBlitFramebuffer", ::glBlitFramebuffer, is_failed);
	resolve_symbol("glRenderbufferStorageMultisample", ::glRenderbufferStorageMultisample, is_failed);
	resolve_symbol("glFramebufferTextureLayer", ::glFramebufferTextureLayer, is_failed);

	return !is_failed;
}

bool OglExtensionManagerImpl::resolve_arb_map_buffer_range()
{
	auto is_failed = false;

	resolve_symbol("glMapBufferRange", ::glMapBufferRange, is_failed);
	resolve_symbol("glFlushMappedBufferRange", ::glFlushMappedBufferRange, is_failed);

	return !is_failed;
}

bool OglExtensionManagerImpl::resolve_arb_uniform_buffer_object()
{
	auto is_failed = false;

	resolve_symbol("glGetUniformIndices", ::glGetUniformIndices, is_failed);
	resolve_symbol("glGetActiveUniformsiv", ::glGetActiveUniformsiv, is_failed);
	resolve_symbol("glGetActiveUniformName", ::glGetActiveUniformName, is_failed);
	resolve_symbol("glGetUniformBlockIndex", ::glGetUniformBlockIndex, is_failed);
	resolve_symbol("glGetActiveUniformBlockiv", ::glGetActiveUniformBlockiv, is_failed);
	resolve_symbol("glGetActiveUniformBlockName", ::glGetActiveUniformBlockName, is_failed);
	resolve_symbol("glUniformBlockBinding", ::glUniformBlockBinding, is_failed);
	resolve_symbol("glBindBufferRange", ::glBindBufferRange, is_failed);
	resolve_symbol("glBindBufferBase", ::glBindBufferBase, is_failed);
	resolve_symbol("glGetIntegeri_v", ::glGetIntegeri_v, is_failed);

	return !is_failed;
}

bool OglExtensionManagerImpl::resolve_arb_vertex_array_object()
{
	auto is_failed = false;

	resolve_symbol("glBindVertexArray", ::glBindVertexArray, is_failed);
	resolve_symbol("glDeleteVertexArrays", ::glDeleteVertexArrays, is_failed);
	resolve_symbol("glGenVertexArrays", ::glGenVertexArrays, is_failed);
	resolve_symbol("glIsVertexArray", ::glIsVertexArray, is_failed);

	return !is_failed;
}

bool OglExtensionManagerImpl::resolve_arb_vertex_buffer_object()
{
	auto is_failed = false;

	resolve_symbol("glBindBufferARB", ::glBindBufferARB, is_failed);
	resolve_symbol("glDeleteBuffersARB", ::glDeleteBuffersARB, is_failed);
	resolve_symbol("glGenBuffersARB", ::glGenBuffersARB, is_failed);
	resolve_symbol("glIsBufferARB", ::glIsBufferARB, is_failed);
	resolve_symbol("glBufferDataARB", ::glBufferDataARB, is_failed);
	resolve_symbol("glBufferSubDataARB", ::glBufferSubDataARB, is_failed);
	resolve_symbol("glGetBufferSubDataARB", ::glGetBufferSubDataARB, is_failed);
	resolve_symbol("glMapBufferARB", ::glMapBufferARB, is_failed);
	resolve_symbol("glUnmapBufferARB", ::glUnmapBufferARB, is_failed);
	resolve_symbol("glGetBufferParameterivARB", ::glGetBufferParameterivARB, is_failed);
	resolve_symbol("glGetBufferPointervARB", ::glGetBufferPointervARB, is_failed);

	return !is_failed;
}

bool OglExtensionManagerImpl::resolve_ext_framebuffer_blit()
{
	auto is_failed = false;

	resolve_symbol("glBlitFramebufferEXT", ::glBlitFramebufferEXT, is_failed);

	return !is_failed;
}

bool OglExtensionManagerImpl::resolve_ext_framebuffer_multisample()
{
	auto is_failed = false;

	resolve_symbol("glRenderbufferStorageMultisampleEXT", ::glRenderbufferStorageMultisampleEXT, is_failed);

	return !is_failed;
}

bool OglExtensionManagerImpl::resolve_ext_framebuffer_object()
{
	auto is_failed = false;

	resolve_symbol("glIsRenderbufferEXT", ::glIsRenderbufferEXT, is_failed);
	resolve_symbol("glBindRenderbufferEXT", ::glBindRenderbufferEXT, is_failed);
	resolve_symbol("glDeleteRenderbuffersEXT", ::glDeleteRenderbuffersEXT, is_failed);
	resolve_symbol("glGenRenderbuffersEXT", ::glGenRenderbuffersEXT, is_failed);
	resolve_symbol("glRenderbufferStorageEXT", ::glRenderbufferStorageEXT, is_failed);
	resolve_symbol("glGetRenderbufferParameterivEXT", ::glGetRenderbufferParameterivEXT, is_failed);
	resolve_symbol("glIsFramebufferEXT", ::glIsFramebufferEXT, is_failed);
	resolve_symbol("glBindFramebufferEXT", ::glBindFramebufferEXT, is_failed);
	resolve_symbol("glDeleteFramebuffersEXT", ::glDeleteFramebuffersEXT, is_failed);
	resolve_symbol("glGenFramebuffersEXT", ::glGenFramebuffersEXT, is_failed);
	resolve_symbol("glCheckFramebufferStatusEXT", ::glCheckFramebufferStatusEXT, is_failed);
	resolve_symbol("glFramebufferTexture1DEXT", ::glFramebufferTexture1DEXT, is_failed);
	resolve_symbol("glFramebufferTexture2DEXT", ::glFramebufferTexture2DEXT, is_failed);
	resolve_symbol("glFramebufferTexture3DEXT", ::glFramebufferTexture3DEXT, is_failed);
	resolve_symbol("glFramebufferRenderbufferEXT", ::glFramebufferRenderbufferEXT, is_failed);
	resolve_symbol("glGetFramebufferAttachmentParameterivEXT", ::glGetFramebufferAttachmentParameterivEXT, is_failed);
	resolve_symbol("glGenerateMipmapEXT", ::glGenerateMipmapEXT, is_failed);

	return !is_failed;
}

//
// OglExtensionManagerImpl
// ==========================================================================


// ==========================================================================
// OglExtensionManager
//

OglExtensionManager::OglExtensionManager()
{
}

OglExtensionManager::~OglExtensionManager()
{
}

//
// OglExtensionManager
// ==========================================================================


// ==========================================================================
// OglExtensionManagerFactory
//

OglExtensionManagerUPtr OglExtensionManagerFactory::create()
{
	auto result = OglExtensionManagerImplUPtr{new OglExtensionManagerImpl{}};

	if (!result->initialize())
	{
		return nullptr;
	}

	return result;
}

//
// OglExtensionManagerFactory
// ==========================================================================


} // detail
} // bstone
