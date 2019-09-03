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
#include "bstone_exception.h"
#include "bstone_sdl_types.h"
#include "bstone_detail_ogl_renderer_utils.h"


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

	~OglExtensionManagerImpl() override;


	int get_count() const noexcept override;

	const std::string& get_name(
		const int extension_index) const noexcept override;


	void probe(
		const OglExtensionId extension_id) override;


	bool has(
		const OglExtensionId extension_id) const noexcept override;

	bool operator[](
		const OglExtensionId extension_id) const noexcept override;


private:
	using ExtensionNames = std::vector<std::string>;

	using Dependencies = std::vector<OglExtensionId>;
	using ResolveSymbolsFunction = void (OglExtensionManagerImpl::*)();


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


	OglContextKind context_kind_;
	int context_major_version_;
	int context_minor_version_;
	ExtensionNames extension_names_;
	Registry registry_;


	static const std::string& get_empty_extension_name() noexcept;

	static int get_registered_extension_count() noexcept;

	static int get_extension_index(
		const OglExtensionId extension_id) noexcept;


	template<typename T>
	static void resolve_symbol(
		const char* const name,
		T& symbol)
	{
		symbol = reinterpret_cast<T>(OglRendererUtils::resolve_symbol(name));

		if (!symbol)
		{
			throw Exception{name};
		}
	}


	void initialize();

	void initialize_registry();


	void get_context_attributes();


	void get_core_extension_names();

	void get_compatibility_extension_names();

	void get_extension_names();


	bool is_gl() const noexcept;

	bool is_glcore() const noexcept;

	bool is_gles1() const noexcept;

	bool is_gles2() const noexcept;

	bool is_extension_compatible(
		const OglExtensionId extension_id) const;


	void probe_generic(
		const OglExtensionId extension_id);


	void resolve_essentials();


	void resolve_v1_0();

	void resolve_v1_1();

	void resolve_v1_2();

	void resolve_v1_3();

	void resolve_v1_4();

	void resolve_v1_5();


	void resolve_v2_0();

	void resolve_v2_1();


	void resolve_v3_0();

	void resolve_v3_1();


	void resolve_arb_buffer_storage();

	void resolve_arb_color_buffer_float();

	void resolve_arb_copy_buffer();

	void resolve_arb_direct_state_access();

	void resolve_arb_framebuffer_object();

	void resolve_arb_map_buffer_range();

	void resolve_arb_sampler_objects();

	void resolve_arb_separate_shader_objects();

	void resolve_arb_uniform_buffer_object();

	void resolve_arb_vertex_array_object();

	void resolve_arb_vertex_buffer_object();


	void resolve_ext_framebuffer_blit();

	void resolve_ext_framebuffer_multisample();

	void resolve_ext_framebuffer_object();
}; // OglExtensionManagerImpl


using OglExtensionManagerImplPtr = OglExtensionManagerImpl*;
using OglExtensionManagerImplUPtr = std::unique_ptr<OglExtensionManagerImpl>;


OglExtensionManagerImpl::OglExtensionManagerImpl()
	:
	context_kind_{},
	extension_names_{},
	registry_{}
{
	initialize();
}

OglExtensionManagerImpl::~OglExtensionManagerImpl()
{
}

void OglExtensionManagerImpl::initialize()
{
	get_context_attributes();
	get_extension_names();

	initialize_registry();
}

int OglExtensionManagerImpl::get_count() const noexcept
{
	return static_cast<int>(extension_names_.size());
}

const std::string& OglExtensionManagerImpl::get_name(
	const int extension_index) const noexcept
{
	if (extension_index < 0 || extension_index >= get_count())
	{
		return get_empty_extension_name();
	}

	return extension_names_[extension_index];
}

void OglExtensionManagerImpl::probe(
	const OglExtensionId extension_id)
{
	probe_generic(extension_id);
}

bool OglExtensionManagerImpl::has(
	const OglExtensionId extension_id) const noexcept
{
	const auto extension_index = get_extension_index(extension_id);

	if (extension_index < 0)
	{
		return false;
	}

	return registry_[extension_index].is_available_;
}

bool OglExtensionManagerImpl::operator[](
	const OglExtensionId extension_id) const noexcept
{
	return has(extension_id);
}

const std::string& OglExtensionManagerImpl::get_empty_extension_name() noexcept
{
	static const auto result = std::string{};

	return result;
}

int OglExtensionManagerImpl::get_registered_extension_count() noexcept
{
	return static_cast<int>(OglExtensionId::count_);
}

int OglExtensionManagerImpl::get_extension_index(
	const OglExtensionId extension_id) noexcept
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
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::essentials)];
		registry_item.is_virtual_ = true;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = true;
		registry_item.is_gles1_ = true;
		registry_item.is_gles2_ = true;
		registry_item.extension_name_ = "essentials";
		registry_item.resolve_symbols_function_ = &OglExtensionManagerImpl::resolve_essentials;
		registry_item.dependencies_ = {};
	}

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
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::arb_buffer_storage)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = true;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "GL_ARB_buffer_storage";
		registry_item.resolve_symbols_function_ = &OglExtensionManagerImpl::resolve_arb_buffer_storage;
		registry_item.dependencies_ = {};
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
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::arb_direct_state_access)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = true;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "GL_ARB_direct_state_access";
		registry_item.resolve_symbols_function_ = &OglExtensionManagerImpl::resolve_arb_direct_state_access;
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
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::arb_sampler_objects)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = true;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "GL_ARB_sampler_objects";
		registry_item.resolve_symbols_function_ = &OglExtensionManagerImpl::resolve_arb_sampler_objects;
		registry_item.dependencies_ = {};
	}

	{
		auto& registry_item = registry_[static_cast<int>(OglExtensionId::arb_separate_shader_objects)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.is_gl_ = true;
		registry_item.is_glcore_ = true;
		registry_item.is_gles1_ = false;
		registry_item.is_gles2_ = false;
		registry_item.extension_name_ = "GL_ARB_separate_shader_objects";
		registry_item.resolve_symbols_function_ = &OglExtensionManagerImpl::resolve_arb_separate_shader_objects;
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

void OglExtensionManagerImpl::get_context_attributes()
{
	const auto context_type = OglRendererUtils::context_get_kind();

	if (context_type == OglContextKind::invalid)
	{
		throw Exception{"Invalid context kind."};
	}

	OglRendererUtils::context_get_version(context_major_version_, context_minor_version_);

	int major_version = 0;
	int minor_version = 0;

	OglRendererUtils::context_get_version(major_version, minor_version);

	context_kind_ = context_type;
	context_major_version_ = major_version;
	context_minor_version_ = minor_version;
}

void OglExtensionManagerImpl::get_core_extension_names()
{
	auto gl_get_integer_v = PFNGLGETINTEGERVPROC{};
	auto gl_get_string_i = PFNGLGETSTRINGIPROC{};

	try
	{
		resolve_symbol("glGetIntegerv", gl_get_integer_v);
		resolve_symbol("glGetStringi", gl_get_string_i);
	}
	catch (const Exception&)
	{
		std::throw_with_nested(Exception{"Failed to resolve essential symbols."});
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
			throw Exception{"Null extension name."};
		}

		extension_names[i] = reinterpret_cast<const char*>(extension_name);
	}

	extension_names_ = std::move(extension_names);
}

void OglExtensionManagerImpl::get_compatibility_extension_names()
{
	auto gl_get_string = PFNGLGETSTRINGPROC{};

	try
	{
		resolve_symbol("glGetString", gl_get_string);
	}
	catch (const Exception&)
	{
		throw Exception{"Failed to resolve essential symbols."};
	}

	const auto ogl_extensions_c_string = gl_get_string(GL_EXTENSIONS);

	if (ogl_extensions_c_string == nullptr)
	{
		throw Exception{"Null extensions string."};
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

	if (context_kind_ == OglContextKind::es)
	{
		if (context_major_version_ >= 3)
		{
			is_core = true;
		}
	}
	else if (context_kind_ == OglContextKind::core)
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

bool OglExtensionManagerImpl::is_gl() const noexcept
{
	return
		context_kind_ == OglContextKind::none ||
		context_kind_ == OglContextKind::compatibility
	;
}

bool OglExtensionManagerImpl::is_glcore() const noexcept
{
	return
		context_kind_ == OglContextKind::core
	;

}

bool OglExtensionManagerImpl::is_gles1() const noexcept
{
	return
		context_kind_ == OglContextKind::es &&
		context_major_version_ == 1
	;
}

bool OglExtensionManagerImpl::is_gles2() const noexcept
{
	return
		context_kind_ == OglContextKind::es &&
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
		throw Exception{"Invalid extension id."};
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
		throw Exception{"Expected symbols loader for specific version."};
	}

	for (const auto dependency_extension_id : registry_item.dependencies_)
	{
		if (!is_extension_compatible(dependency_extension_id))
		{
			continue;
		}

		probe_generic(dependency_extension_id);

		if (!has(dependency_extension_id))
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
		try
		{
			(this->*registry_item.resolve_symbols_function_)();
		}
		catch (const Exception&)
		{
			return;
		}
	}

	registry_item.is_available_ = true;
}

void OglExtensionManagerImpl::resolve_essentials()
{
	resolve_symbol("glGetError", ::glGetError);
	resolve_symbol("glGetIntegerv", ::glGetIntegerv);
}

void OglExtensionManagerImpl::resolve_v1_0()
{
	resolve_symbol("glCullFace", ::glCullFace);
	resolve_symbol("glFrontFace", ::glFrontFace);
	resolve_symbol("glHint", ::glHint);
	resolve_symbol("glLineWidth", ::glLineWidth);
	resolve_symbol("glPointSize", ::glPointSize);
	resolve_symbol("glPolygonMode", ::glPolygonMode);
	resolve_symbol("glScissor", ::glScissor);
	resolve_symbol("glTexParameterf", ::glTexParameterf);
	resolve_symbol("glTexParameterfv", ::glTexParameterfv);
	resolve_symbol("glTexParameteri", ::glTexParameteri);
	resolve_symbol("glTexParameteriv", ::glTexParameteriv);
	resolve_symbol("glTexImage1D", ::glTexImage1D);
	resolve_symbol("glTexImage2D", ::glTexImage2D);
	resolve_symbol("glDrawBuffer", ::glDrawBuffer);
	resolve_symbol("glClear", ::glClear);
	resolve_symbol("glClearColor", ::glClearColor);
	resolve_symbol("glClearStencil", ::glClearStencil);
	resolve_symbol("glClearDepth", ::glClearDepth);
	resolve_symbol("glStencilMask", ::glStencilMask);
	resolve_symbol("glColorMask", ::glColorMask);
	resolve_symbol("glDepthMask", ::glDepthMask);
	resolve_symbol("glDisable", ::glDisable);
	resolve_symbol("glEnable", ::glEnable);
	resolve_symbol("glFinish", ::glFinish);
	resolve_symbol("glFlush", ::glFlush);
	resolve_symbol("glBlendFunc", ::glBlendFunc);
	resolve_symbol("glLogicOp", ::glLogicOp);
	resolve_symbol("glStencilFunc", ::glStencilFunc);
	resolve_symbol("glStencilOp", ::glStencilOp);
	resolve_symbol("glDepthFunc", ::glDepthFunc);
	resolve_symbol("glPixelStoref", ::glPixelStoref);
	resolve_symbol("glPixelStorei", ::glPixelStorei);
	resolve_symbol("glReadBuffer", ::glReadBuffer);
	resolve_symbol("glReadPixels", ::glReadPixels);
	resolve_symbol("glGetBooleanv", ::glGetBooleanv);
	resolve_symbol("glGetDoublev", ::glGetDoublev);
	resolve_symbol("glGetError", ::glGetError);
	resolve_symbol("glGetFloatv", ::glGetFloatv);
	resolve_symbol("glGetIntegerv", ::glGetIntegerv);
	resolve_symbol("glGetString", ::glGetString);
	resolve_symbol("glGetTexImage", ::glGetTexImage);
	resolve_symbol("glGetTexParameterfv", ::glGetTexParameterfv);
	resolve_symbol("glGetTexParameteriv", ::glGetTexParameteriv);
	resolve_symbol("glGetTexLevelParameterfv", ::glGetTexLevelParameterfv);
	resolve_symbol("glGetTexLevelParameteriv", ::glGetTexLevelParameteriv);
	resolve_symbol("glIsEnabled", ::glIsEnabled);
	resolve_symbol("glDepthRange", ::glDepthRange);
	resolve_symbol("glViewport", ::glViewport);
	resolve_symbol("glNewList", ::glNewList);
	resolve_symbol("glEndList", ::glEndList);
	resolve_symbol("glCallList", ::glCallList);
	resolve_symbol("glCallLists", ::glCallLists);
	resolve_symbol("glDeleteLists", ::glDeleteLists);
	resolve_symbol("glGenLists", ::glGenLists);
	resolve_symbol("glListBase", ::glListBase);
	resolve_symbol("glBegin", ::glBegin);
	resolve_symbol("glBitmap", ::glBitmap);
	resolve_symbol("glColor3b", ::glColor3b);
	resolve_symbol("glColor3bv", ::glColor3bv);
	resolve_symbol("glColor3d", ::glColor3d);
	resolve_symbol("glColor3dv", ::glColor3dv);
	resolve_symbol("glColor3f", ::glColor3f);
	resolve_symbol("glColor3fv", ::glColor3fv);
	resolve_symbol("glColor3i", ::glColor3i);
	resolve_symbol("glColor3iv", ::glColor3iv);
	resolve_symbol("glColor3s", ::glColor3s);
	resolve_symbol("glColor3sv", ::glColor3sv);
	resolve_symbol("glColor3ub", ::glColor3ub);
	resolve_symbol("glColor3ubv", ::glColor3ubv);
	resolve_symbol("glColor3ui", ::glColor3ui);
	resolve_symbol("glColor3uiv", ::glColor3uiv);
	resolve_symbol("glColor3us", ::glColor3us);
	resolve_symbol("glColor3usv", ::glColor3usv);
	resolve_symbol("glColor4b", ::glColor4b);
	resolve_symbol("glColor4bv", ::glColor4bv);
	resolve_symbol("glColor4d", ::glColor4d);
	resolve_symbol("glColor4dv", ::glColor4dv);
	resolve_symbol("glColor4f", ::glColor4f);
	resolve_symbol("glColor4fv", ::glColor4fv);
	resolve_symbol("glColor4i", ::glColor4i);
	resolve_symbol("glColor4iv", ::glColor4iv);
	resolve_symbol("glColor4s", ::glColor4s);
	resolve_symbol("glColor4sv", ::glColor4sv);
	resolve_symbol("glColor4ub", ::glColor4ub);
	resolve_symbol("glColor4ubv", ::glColor4ubv);
	resolve_symbol("glColor4ui", ::glColor4ui);
	resolve_symbol("glColor4uiv", ::glColor4uiv);
	resolve_symbol("glColor4us", ::glColor4us);
	resolve_symbol("glColor4usv", ::glColor4usv);
	resolve_symbol("glEdgeFlag", ::glEdgeFlag);
	resolve_symbol("glEdgeFlagv", ::glEdgeFlagv);
	resolve_symbol("glEnd", ::glEnd);
	resolve_symbol("glIndexd", ::glIndexd);
	resolve_symbol("glIndexdv", ::glIndexdv);
	resolve_symbol("glIndexf", ::glIndexf);
	resolve_symbol("glIndexfv", ::glIndexfv);
	resolve_symbol("glIndexi", ::glIndexi);
	resolve_symbol("glIndexiv", ::glIndexiv);
	resolve_symbol("glIndexs", ::glIndexs);
	resolve_symbol("glIndexsv", ::glIndexsv);
	resolve_symbol("glNormal3b", ::glNormal3b);
	resolve_symbol("glNormal3bv", ::glNormal3bv);
	resolve_symbol("glNormal3d", ::glNormal3d);
	resolve_symbol("glNormal3dv", ::glNormal3dv);
	resolve_symbol("glNormal3f", ::glNormal3f);
	resolve_symbol("glNormal3fv", ::glNormal3fv);
	resolve_symbol("glNormal3i", ::glNormal3i);
	resolve_symbol("glNormal3iv", ::glNormal3iv);
	resolve_symbol("glNormal3s", ::glNormal3s);
	resolve_symbol("glNormal3sv", ::glNormal3sv);
	resolve_symbol("glRasterPos2d", ::glRasterPos2d);
	resolve_symbol("glRasterPos2dv", ::glRasterPos2dv);
	resolve_symbol("glRasterPos2f", ::glRasterPos2f);
	resolve_symbol("glRasterPos2fv", ::glRasterPos2fv);
	resolve_symbol("glRasterPos2i", ::glRasterPos2i);
	resolve_symbol("glRasterPos2iv", ::glRasterPos2iv);
	resolve_symbol("glRasterPos2s", ::glRasterPos2s);
	resolve_symbol("glRasterPos2sv", ::glRasterPos2sv);
	resolve_symbol("glRasterPos3d", ::glRasterPos3d);
	resolve_symbol("glRasterPos3dv", ::glRasterPos3dv);
	resolve_symbol("glRasterPos3f", ::glRasterPos3f);
	resolve_symbol("glRasterPos3fv", ::glRasterPos3fv);
	resolve_symbol("glRasterPos3i", ::glRasterPos3i);
	resolve_symbol("glRasterPos3iv", ::glRasterPos3iv);
	resolve_symbol("glRasterPos3s", ::glRasterPos3s);
	resolve_symbol("glRasterPos3sv", ::glRasterPos3sv);
	resolve_symbol("glRasterPos4d", ::glRasterPos4d);
	resolve_symbol("glRasterPos4dv", ::glRasterPos4dv);
	resolve_symbol("glRasterPos4f", ::glRasterPos4f);
	resolve_symbol("glRasterPos4fv", ::glRasterPos4fv);
	resolve_symbol("glRasterPos4i", ::glRasterPos4i);
	resolve_symbol("glRasterPos4iv", ::glRasterPos4iv);
	resolve_symbol("glRasterPos4s", ::glRasterPos4s);
	resolve_symbol("glRasterPos4sv", ::glRasterPos4sv);
	resolve_symbol("glRectd", ::glRectd);
	resolve_symbol("glRectdv", ::glRectdv);
	resolve_symbol("glRectf", ::glRectf);
	resolve_symbol("glRectfv", ::glRectfv);
	resolve_symbol("glRecti", ::glRecti);
	resolve_symbol("glRectiv", ::glRectiv);
	resolve_symbol("glRects", ::glRects);
	resolve_symbol("glRectsv", ::glRectsv);
	resolve_symbol("glTexCoord1d", ::glTexCoord1d);
	resolve_symbol("glTexCoord1dv", ::glTexCoord1dv);
	resolve_symbol("glTexCoord1f", ::glTexCoord1f);
	resolve_symbol("glTexCoord1fv", ::glTexCoord1fv);
	resolve_symbol("glTexCoord1i", ::glTexCoord1i);
	resolve_symbol("glTexCoord1iv", ::glTexCoord1iv);
	resolve_symbol("glTexCoord1s", ::glTexCoord1s);
	resolve_symbol("glTexCoord1sv", ::glTexCoord1sv);
	resolve_symbol("glTexCoord2d", ::glTexCoord2d);
	resolve_symbol("glTexCoord2dv", ::glTexCoord2dv);
	resolve_symbol("glTexCoord2f", ::glTexCoord2f);
	resolve_symbol("glTexCoord2fv", ::glTexCoord2fv);
	resolve_symbol("glTexCoord2i", ::glTexCoord2i);
	resolve_symbol("glTexCoord2iv", ::glTexCoord2iv);
	resolve_symbol("glTexCoord2s", ::glTexCoord2s);
	resolve_symbol("glTexCoord2sv", ::glTexCoord2sv);
	resolve_symbol("glTexCoord3d", ::glTexCoord3d);
	resolve_symbol("glTexCoord3dv", ::glTexCoord3dv);
	resolve_symbol("glTexCoord3f", ::glTexCoord3f);
	resolve_symbol("glTexCoord3fv", ::glTexCoord3fv);
	resolve_symbol("glTexCoord3i", ::glTexCoord3i);
	resolve_symbol("glTexCoord3iv", ::glTexCoord3iv);
	resolve_symbol("glTexCoord3s", ::glTexCoord3s);
	resolve_symbol("glTexCoord3sv", ::glTexCoord3sv);
	resolve_symbol("glTexCoord4d", ::glTexCoord4d);
	resolve_symbol("glTexCoord4dv", ::glTexCoord4dv);
	resolve_symbol("glTexCoord4f", ::glTexCoord4f);
	resolve_symbol("glTexCoord4fv", ::glTexCoord4fv);
	resolve_symbol("glTexCoord4i", ::glTexCoord4i);
	resolve_symbol("glTexCoord4iv", ::glTexCoord4iv);
	resolve_symbol("glTexCoord4s", ::glTexCoord4s);
	resolve_symbol("glTexCoord4sv", ::glTexCoord4sv);
	resolve_symbol("glVertex2d", ::glVertex2d);
	resolve_symbol("glVertex2dv", ::glVertex2dv);
	resolve_symbol("glVertex2f", ::glVertex2f);
	resolve_symbol("glVertex2fv", ::glVertex2fv);
	resolve_symbol("glVertex2i", ::glVertex2i);
	resolve_symbol("glVertex2iv", ::glVertex2iv);
	resolve_symbol("glVertex2s", ::glVertex2s);
	resolve_symbol("glVertex2sv", ::glVertex2sv);
	resolve_symbol("glVertex3d", ::glVertex3d);
	resolve_symbol("glVertex3dv", ::glVertex3dv);
	resolve_symbol("glVertex3f", ::glVertex3f);
	resolve_symbol("glVertex3fv", ::glVertex3fv);
	resolve_symbol("glVertex3i", ::glVertex3i);
	resolve_symbol("glVertex3iv", ::glVertex3iv);
	resolve_symbol("glVertex3s", ::glVertex3s);
	resolve_symbol("glVertex3sv", ::glVertex3sv);
	resolve_symbol("glVertex4d", ::glVertex4d);
	resolve_symbol("glVertex4dv", ::glVertex4dv);
	resolve_symbol("glVertex4f", ::glVertex4f);
	resolve_symbol("glVertex4fv", ::glVertex4fv);
	resolve_symbol("glVertex4i", ::glVertex4i);
	resolve_symbol("glVertex4iv", ::glVertex4iv);
	resolve_symbol("glVertex4s", ::glVertex4s);
	resolve_symbol("glVertex4sv", ::glVertex4sv);
	resolve_symbol("glClipPlane", ::glClipPlane);
	resolve_symbol("glColorMaterial", ::glColorMaterial);
	resolve_symbol("glFogf", ::glFogf);
	resolve_symbol("glFogfv", ::glFogfv);
	resolve_symbol("glFogi", ::glFogi);
	resolve_symbol("glFogiv", ::glFogiv);
	resolve_symbol("glLightf", ::glLightf);
	resolve_symbol("glLightfv", ::glLightfv);
	resolve_symbol("glLighti", ::glLighti);
	resolve_symbol("glLightiv", ::glLightiv);
	resolve_symbol("glLightModelf", ::glLightModelf);
	resolve_symbol("glLightModelfv", ::glLightModelfv);
	resolve_symbol("glLightModeli", ::glLightModeli);
	resolve_symbol("glLightModeliv", ::glLightModeliv);
	resolve_symbol("glLineStipple", ::glLineStipple);
	resolve_symbol("glMaterialf", ::glMaterialf);
	resolve_symbol("glMaterialfv", ::glMaterialfv);
	resolve_symbol("glMateriali", ::glMateriali);
	resolve_symbol("glMaterialiv", ::glMaterialiv);
	resolve_symbol("glPolygonStipple", ::glPolygonStipple);
	resolve_symbol("glShadeModel", ::glShadeModel);
	resolve_symbol("glTexEnvf", ::glTexEnvf);
	resolve_symbol("glTexEnvfv", ::glTexEnvfv);
	resolve_symbol("glTexEnvi", ::glTexEnvi);
	resolve_symbol("glTexEnviv", ::glTexEnviv);
	resolve_symbol("glTexGend", ::glTexGend);
	resolve_symbol("glTexGendv", ::glTexGendv);
	resolve_symbol("glTexGenf", ::glTexGenf);
	resolve_symbol("glTexGenfv", ::glTexGenfv);
	resolve_symbol("glTexGeni", ::glTexGeni);
	resolve_symbol("glTexGeniv", ::glTexGeniv);
	resolve_symbol("glFeedbackBuffer", ::glFeedbackBuffer);
	resolve_symbol("glSelectBuffer", ::glSelectBuffer);
	resolve_symbol("glRenderMode", ::glRenderMode);
	resolve_symbol("glInitNames", ::glInitNames);
	resolve_symbol("glLoadName", ::glLoadName);
	resolve_symbol("glPassThrough", ::glPassThrough);
	resolve_symbol("glPopName", ::glPopName);
	resolve_symbol("glPushName", ::glPushName);
	resolve_symbol("glClearAccum", ::glClearAccum);
	resolve_symbol("glClearIndex", ::glClearIndex);
	resolve_symbol("glIndexMask", ::glIndexMask);
	resolve_symbol("glAccum", ::glAccum);
	resolve_symbol("glPopAttrib", ::glPopAttrib);
	resolve_symbol("glPushAttrib", ::glPushAttrib);
	resolve_symbol("glMap1d", ::glMap1d);
	resolve_symbol("glMap1f", ::glMap1f);
	resolve_symbol("glMap2d", ::glMap2d);
	resolve_symbol("glMap2f", ::glMap2f);
	resolve_symbol("glMapGrid1d", ::glMapGrid1d);
	resolve_symbol("glMapGrid1f", ::glMapGrid1f);
	resolve_symbol("glMapGrid2d", ::glMapGrid2d);
	resolve_symbol("glMapGrid2f", ::glMapGrid2f);
	resolve_symbol("glEvalCoord1d", ::glEvalCoord1d);
	resolve_symbol("glEvalCoord1dv", ::glEvalCoord1dv);
	resolve_symbol("glEvalCoord1f", ::glEvalCoord1f);
	resolve_symbol("glEvalCoord1fv", ::glEvalCoord1fv);
	resolve_symbol("glEvalCoord2d", ::glEvalCoord2d);
	resolve_symbol("glEvalCoord2dv", ::glEvalCoord2dv);
	resolve_symbol("glEvalCoord2f", ::glEvalCoord2f);
	resolve_symbol("glEvalCoord2fv", ::glEvalCoord2fv);
	resolve_symbol("glEvalMesh1", ::glEvalMesh1);
	resolve_symbol("glEvalPoint1", ::glEvalPoint1);
	resolve_symbol("glEvalMesh2", ::glEvalMesh2);
	resolve_symbol("glEvalPoint2", ::glEvalPoint2);
	resolve_symbol("glAlphaFunc", ::glAlphaFunc);
	resolve_symbol("glPixelZoom", ::glPixelZoom);
	resolve_symbol("glPixelTransferf", ::glPixelTransferf);
	resolve_symbol("glPixelTransferi", ::glPixelTransferi);
	resolve_symbol("glPixelMapfv", ::glPixelMapfv);
	resolve_symbol("glPixelMapuiv", ::glPixelMapuiv);
	resolve_symbol("glPixelMapusv", ::glPixelMapusv);
	resolve_symbol("glCopyPixels", ::glCopyPixels);
	resolve_symbol("glDrawPixels", ::glDrawPixels);
	resolve_symbol("glGetClipPlane", ::glGetClipPlane);
	resolve_symbol("glGetLightfv", ::glGetLightfv);
	resolve_symbol("glGetLightiv", ::glGetLightiv);
	resolve_symbol("glGetMapdv", ::glGetMapdv);
	resolve_symbol("glGetMapfv", ::glGetMapfv);
	resolve_symbol("glGetMapiv", ::glGetMapiv);
	resolve_symbol("glGetMaterialfv", ::glGetMaterialfv);
	resolve_symbol("glGetMaterialiv", ::glGetMaterialiv);
	resolve_symbol("glGetPixelMapfv", ::glGetPixelMapfv);
	resolve_symbol("glGetPixelMapuiv", ::glGetPixelMapuiv);
	resolve_symbol("glGetPixelMapusv", ::glGetPixelMapusv);
	resolve_symbol("glGetPolygonStipple", ::glGetPolygonStipple);
	resolve_symbol("glGetTexEnvfv", ::glGetTexEnvfv);
	resolve_symbol("glGetTexEnviv", ::glGetTexEnviv);
	resolve_symbol("glGetTexGendv", ::glGetTexGendv);
	resolve_symbol("glGetTexGenfv", ::glGetTexGenfv);
	resolve_symbol("glGetTexGeniv", ::glGetTexGeniv);
	resolve_symbol("glIsList", ::glIsList);
	resolve_symbol("glFrustum", ::glFrustum);
	resolve_symbol("glLoadIdentity", ::glLoadIdentity);
	resolve_symbol("glLoadMatrixf", ::glLoadMatrixf);
	resolve_symbol("glLoadMatrixd", ::glLoadMatrixd);
	resolve_symbol("glMatrixMode", ::glMatrixMode);
	resolve_symbol("glMultMatrixf", ::glMultMatrixf);
	resolve_symbol("glMultMatrixd", ::glMultMatrixd);
	resolve_symbol("glOrtho", ::glOrtho);
	resolve_symbol("glPopMatrix", ::glPopMatrix);
	resolve_symbol("glPushMatrix", ::glPushMatrix);
	resolve_symbol("glRotated", ::glRotated);
	resolve_symbol("glRotatef", ::glRotatef);
	resolve_symbol("glScaled", ::glScaled);
	resolve_symbol("glScalef", ::glScalef);
	resolve_symbol("glTranslated", ::glTranslated);
	resolve_symbol("glTranslatef", ::glTranslatef);
}

void OglExtensionManagerImpl::resolve_v1_1()
{
	resolve_symbol("glDrawArrays", ::glDrawArrays);
	resolve_symbol("glDrawElements", ::glDrawElements);
	resolve_symbol("glGetPointerv", ::glGetPointerv);
	resolve_symbol("glPolygonOffset", ::glPolygonOffset);
	resolve_symbol("glCopyTexImage1D", ::glCopyTexImage1D);
	resolve_symbol("glCopyTexImage2D", ::glCopyTexImage2D);
	resolve_symbol("glCopyTexSubImage1D", ::glCopyTexSubImage1D);
	resolve_symbol("glCopyTexSubImage2D", ::glCopyTexSubImage2D);
	resolve_symbol("glTexSubImage1D", ::glTexSubImage1D);
	resolve_symbol("glTexSubImage2D", ::glTexSubImage2D);
	resolve_symbol("glBindTexture", ::glBindTexture);
	resolve_symbol("glDeleteTextures", ::glDeleteTextures);
	resolve_symbol("glGenTextures", ::glGenTextures);
	resolve_symbol("glIsTexture", ::glIsTexture);
	resolve_symbol("glArrayElement", ::glArrayElement);
	resolve_symbol("glColorPointer", ::glColorPointer);
	resolve_symbol("glDisableClientState", ::glDisableClientState);
	resolve_symbol("glEdgeFlagPointer", ::glEdgeFlagPointer);
	resolve_symbol("glEnableClientState", ::glEnableClientState);
	resolve_symbol("glIndexPointer", ::glIndexPointer);
	resolve_symbol("glInterleavedArrays", ::glInterleavedArrays);
	resolve_symbol("glNormalPointer", ::glNormalPointer);
	resolve_symbol("glTexCoordPointer", ::glTexCoordPointer);
	resolve_symbol("glVertexPointer", ::glVertexPointer);
	resolve_symbol("glAreTexturesResident", ::glAreTexturesResident);
	resolve_symbol("glPrioritizeTextures", ::glPrioritizeTextures);
	resolve_symbol("glIndexub", ::glIndexub);
	resolve_symbol("glIndexubv", ::glIndexubv);
	resolve_symbol("glPopClientAttrib", ::glPopClientAttrib);
	resolve_symbol("glPushClientAttrib", ::glPushClientAttrib);
}

void OglExtensionManagerImpl::resolve_v1_2()
{
	resolve_symbol("glDrawRangeElements", ::glDrawRangeElements);
	resolve_symbol("glTexImage3D", ::glTexImage3D);
	resolve_symbol("glTexSubImage3D", ::glTexSubImage3D);
	resolve_symbol("glCopyTexSubImage3D", ::glCopyTexSubImage3D);
}

void OglExtensionManagerImpl::resolve_v1_3()
{
	resolve_symbol("glActiveTexture", ::glActiveTexture);
	resolve_symbol("glSampleCoverage", ::glSampleCoverage);
	resolve_symbol("glCompressedTexImage3D", ::glCompressedTexImage3D);
	resolve_symbol("glCompressedTexImage2D", ::glCompressedTexImage2D);
	resolve_symbol("glCompressedTexImage1D", ::glCompressedTexImage1D);
	resolve_symbol("glCompressedTexSubImage3D", ::glCompressedTexSubImage3D);
	resolve_symbol("glCompressedTexSubImage2D", ::glCompressedTexSubImage2D);
	resolve_symbol("glCompressedTexSubImage1D", ::glCompressedTexSubImage1D);
	resolve_symbol("glGetCompressedTexImage", ::glGetCompressedTexImage);
	resolve_symbol("glClientActiveTexture", ::glClientActiveTexture);
	resolve_symbol("glMultiTexCoord1d", ::glMultiTexCoord1d);
	resolve_symbol("glMultiTexCoord1dv", ::glMultiTexCoord1dv);
	resolve_symbol("glMultiTexCoord1f", ::glMultiTexCoord1f);
	resolve_symbol("glMultiTexCoord1fv", ::glMultiTexCoord1fv);
	resolve_symbol("glMultiTexCoord1i", ::glMultiTexCoord1i);
	resolve_symbol("glMultiTexCoord1iv", ::glMultiTexCoord1iv);
	resolve_symbol("glMultiTexCoord1s", ::glMultiTexCoord1s);
	resolve_symbol("glMultiTexCoord1sv", ::glMultiTexCoord1sv);
	resolve_symbol("glMultiTexCoord2d", ::glMultiTexCoord2d);
	resolve_symbol("glMultiTexCoord2dv", ::glMultiTexCoord2dv);
	resolve_symbol("glMultiTexCoord2f", ::glMultiTexCoord2f);
	resolve_symbol("glMultiTexCoord2fv", ::glMultiTexCoord2fv);
	resolve_symbol("glMultiTexCoord2i", ::glMultiTexCoord2i);
	resolve_symbol("glMultiTexCoord2iv", ::glMultiTexCoord2iv);
	resolve_symbol("glMultiTexCoord2s", ::glMultiTexCoord2s);
	resolve_symbol("glMultiTexCoord2sv", ::glMultiTexCoord2sv);
	resolve_symbol("glMultiTexCoord3d", ::glMultiTexCoord3d);
	resolve_symbol("glMultiTexCoord3dv", ::glMultiTexCoord3dv);
	resolve_symbol("glMultiTexCoord3f", ::glMultiTexCoord3f);
	resolve_symbol("glMultiTexCoord3fv", ::glMultiTexCoord3fv);
	resolve_symbol("glMultiTexCoord3i", ::glMultiTexCoord3i);
	resolve_symbol("glMultiTexCoord3iv", ::glMultiTexCoord3iv);
	resolve_symbol("glMultiTexCoord3s", ::glMultiTexCoord3s);
	resolve_symbol("glMultiTexCoord3sv", ::glMultiTexCoord3sv);
	resolve_symbol("glMultiTexCoord4d", ::glMultiTexCoord4d);
	resolve_symbol("glMultiTexCoord4dv", ::glMultiTexCoord4dv);
	resolve_symbol("glMultiTexCoord4f", ::glMultiTexCoord4f);
	resolve_symbol("glMultiTexCoord4fv", ::glMultiTexCoord4fv);
	resolve_symbol("glMultiTexCoord4i", ::glMultiTexCoord4i);
	resolve_symbol("glMultiTexCoord4iv", ::glMultiTexCoord4iv);
	resolve_symbol("glMultiTexCoord4s", ::glMultiTexCoord4s);
	resolve_symbol("glMultiTexCoord4sv", ::glMultiTexCoord4sv);
	resolve_symbol("glLoadTransposeMatrixf", ::glLoadTransposeMatrixf);
	resolve_symbol("glLoadTransposeMatrixd", ::glLoadTransposeMatrixd);
	resolve_symbol("glMultTransposeMatrixf", ::glMultTransposeMatrixf);
	resolve_symbol("glMultTransposeMatrixd", ::glMultTransposeMatrixd);
}

void OglExtensionManagerImpl::resolve_v1_4()
{
	resolve_symbol("glBlendFuncSeparate", ::glBlendFuncSeparate);
	resolve_symbol("glMultiDrawArrays", ::glMultiDrawArrays);
	resolve_symbol("glMultiDrawElements", ::glMultiDrawElements);
	resolve_symbol("glPointParameterf", ::glPointParameterf);
	resolve_symbol("glPointParameterfv", ::glPointParameterfv);
	resolve_symbol("glPointParameteri", ::glPointParameteri);
	resolve_symbol("glPointParameteriv", ::glPointParameteriv);
	resolve_symbol("glFogCoordf", ::glFogCoordf);
	resolve_symbol("glFogCoordfv", ::glFogCoordfv);
	resolve_symbol("glFogCoordd", ::glFogCoordd);
	resolve_symbol("glFogCoorddv", ::glFogCoorddv);
	resolve_symbol("glFogCoordPointer", ::glFogCoordPointer);
	resolve_symbol("glSecondaryColor3b", ::glSecondaryColor3b);
	resolve_symbol("glSecondaryColor3bv", ::glSecondaryColor3bv);
	resolve_symbol("glSecondaryColor3d", ::glSecondaryColor3d);
	resolve_symbol("glSecondaryColor3dv", ::glSecondaryColor3dv);
	resolve_symbol("glSecondaryColor3f", ::glSecondaryColor3f);
	resolve_symbol("glSecondaryColor3fv", ::glSecondaryColor3fv);
	resolve_symbol("glSecondaryColor3i", ::glSecondaryColor3i);
	resolve_symbol("glSecondaryColor3iv", ::glSecondaryColor3iv);
	resolve_symbol("glSecondaryColor3s", ::glSecondaryColor3s);
	resolve_symbol("glSecondaryColor3sv", ::glSecondaryColor3sv);
	resolve_symbol("glSecondaryColor3ub", ::glSecondaryColor3ub);
	resolve_symbol("glSecondaryColor3ubv", ::glSecondaryColor3ubv);
	resolve_symbol("glSecondaryColor3ui", ::glSecondaryColor3ui);
	resolve_symbol("glSecondaryColor3uiv", ::glSecondaryColor3uiv);
	resolve_symbol("glSecondaryColor3us", ::glSecondaryColor3us);
	resolve_symbol("glSecondaryColor3usv", ::glSecondaryColor3usv);
	resolve_symbol("glSecondaryColorPointer", ::glSecondaryColorPointer);
	resolve_symbol("glWindowPos2d", ::glWindowPos2d);
	resolve_symbol("glWindowPos2dv", ::glWindowPos2dv);
	resolve_symbol("glWindowPos2f", ::glWindowPos2f);
	resolve_symbol("glWindowPos2fv", ::glWindowPos2fv);
	resolve_symbol("glWindowPos2i", ::glWindowPos2i);
	resolve_symbol("glWindowPos2iv", ::glWindowPos2iv);
	resolve_symbol("glWindowPos2s", ::glWindowPos2s);
	resolve_symbol("glWindowPos2sv", ::glWindowPos2sv);
	resolve_symbol("glWindowPos3d", ::glWindowPos3d);
	resolve_symbol("glWindowPos3dv", ::glWindowPos3dv);
	resolve_symbol("glWindowPos3f", ::glWindowPos3f);
	resolve_symbol("glWindowPos3fv", ::glWindowPos3fv);
	resolve_symbol("glWindowPos3i", ::glWindowPos3i);
	resolve_symbol("glWindowPos3iv", ::glWindowPos3iv);
	resolve_symbol("glWindowPos3s", ::glWindowPos3s);
	resolve_symbol("glWindowPos3sv", ::glWindowPos3sv);
	resolve_symbol("glBlendColor", ::glBlendColor);
	resolve_symbol("glBlendEquation", ::glBlendEquation);
}

void OglExtensionManagerImpl::resolve_v1_5()
{
	resolve_symbol("glGenQueries", ::glGenQueries);
	resolve_symbol("glDeleteQueries", ::glDeleteQueries);
	resolve_symbol("glIsQuery", ::glIsQuery);
	resolve_symbol("glBeginQuery", ::glBeginQuery);
	resolve_symbol("glEndQuery", ::glEndQuery);
	resolve_symbol("glGetQueryiv", ::glGetQueryiv);
	resolve_symbol("glGetQueryObjectiv", ::glGetQueryObjectiv);
	resolve_symbol("glGetQueryObjectuiv", ::glGetQueryObjectuiv);
	resolve_symbol("glBindBuffer", ::glBindBuffer);
	resolve_symbol("glDeleteBuffers", ::glDeleteBuffers);
	resolve_symbol("glGenBuffers", ::glGenBuffers);
	resolve_symbol("glIsBuffer", ::glIsBuffer);
	resolve_symbol("glBufferData", ::glBufferData);
	resolve_symbol("glBufferSubData", ::glBufferSubData);
	resolve_symbol("glGetBufferSubData", ::glGetBufferSubData);
	resolve_symbol("glMapBuffer", ::glMapBuffer);
	resolve_symbol("glUnmapBuffer", ::glUnmapBuffer);
	resolve_symbol("glGetBufferParameteriv", ::glGetBufferParameteriv);
	resolve_symbol("glGetBufferPointerv", ::glGetBufferPointerv);
}

void OglExtensionManagerImpl::resolve_v2_0()
{
	resolve_symbol("glBlendEquationSeparate", ::glBlendEquationSeparate);
	resolve_symbol("glDrawBuffers", ::glDrawBuffers);
	resolve_symbol("glStencilOpSeparate", ::glStencilOpSeparate);
	resolve_symbol("glStencilFuncSeparate", ::glStencilFuncSeparate);
	resolve_symbol("glStencilMaskSeparate", ::glStencilMaskSeparate);
	resolve_symbol("glAttachShader", ::glAttachShader);
	resolve_symbol("glBindAttribLocation", ::glBindAttribLocation);
	resolve_symbol("glCompileShader", ::glCompileShader);
	resolve_symbol("glCreateProgram", ::glCreateProgram);
	resolve_symbol("glCreateShader", ::glCreateShader);
	resolve_symbol("glDeleteProgram", ::glDeleteProgram);
	resolve_symbol("glDeleteShader", ::glDeleteShader);
	resolve_symbol("glDetachShader", ::glDetachShader);
	resolve_symbol("glDisableVertexAttribArray", ::glDisableVertexAttribArray);
	resolve_symbol("glEnableVertexAttribArray", ::glEnableVertexAttribArray);
	resolve_symbol("glGetActiveAttrib", ::glGetActiveAttrib);
	resolve_symbol("glGetActiveUniform", ::glGetActiveUniform);
	resolve_symbol("glGetAttachedShaders", ::glGetAttachedShaders);
	resolve_symbol("glGetAttribLocation", ::glGetAttribLocation);
	resolve_symbol("glGetProgramiv", ::glGetProgramiv);
	resolve_symbol("glGetProgramInfoLog", ::glGetProgramInfoLog);
	resolve_symbol("glGetShaderiv", ::glGetShaderiv);
	resolve_symbol("glGetShaderInfoLog", ::glGetShaderInfoLog);
	resolve_symbol("glGetShaderSource", ::glGetShaderSource);
	resolve_symbol("glGetUniformLocation", ::glGetUniformLocation);
	resolve_symbol("glGetUniformfv", ::glGetUniformfv);
	resolve_symbol("glGetUniformiv", ::glGetUniformiv);
	resolve_symbol("glGetVertexAttribdv", ::glGetVertexAttribdv);
	resolve_symbol("glGetVertexAttribfv", ::glGetVertexAttribfv);
	resolve_symbol("glGetVertexAttribiv", ::glGetVertexAttribiv);
	resolve_symbol("glGetVertexAttribPointerv", ::glGetVertexAttribPointerv);
	resolve_symbol("glIsProgram", ::glIsProgram);
	resolve_symbol("glIsShader", ::glIsShader);
	resolve_symbol("glLinkProgram", ::glLinkProgram);
	resolve_symbol("glShaderSource", ::glShaderSource);
	resolve_symbol("glUseProgram", ::glUseProgram);
	resolve_symbol("glUniform1f", ::glUniform1f);
	resolve_symbol("glUniform2f", ::glUniform2f);
	resolve_symbol("glUniform3f", ::glUniform3f);
	resolve_symbol("glUniform4f", ::glUniform4f);
	resolve_symbol("glUniform1i", ::glUniform1i);
	resolve_symbol("glUniform2i", ::glUniform2i);
	resolve_symbol("glUniform3i", ::glUniform3i);
	resolve_symbol("glUniform4i", ::glUniform4i);
	resolve_symbol("glUniform1fv", ::glUniform1fv);
	resolve_symbol("glUniform2fv", ::glUniform2fv);
	resolve_symbol("glUniform3fv", ::glUniform3fv);
	resolve_symbol("glUniform4fv", ::glUniform4fv);
	resolve_symbol("glUniform1iv", ::glUniform1iv);
	resolve_symbol("glUniform2iv", ::glUniform2iv);
	resolve_symbol("glUniform3iv", ::glUniform3iv);
	resolve_symbol("glUniform4iv", ::glUniform4iv);
	resolve_symbol("glUniformMatrix2fv", ::glUniformMatrix2fv);
	resolve_symbol("glUniformMatrix3fv", ::glUniformMatrix3fv);
	resolve_symbol("glUniformMatrix4fv", ::glUniformMatrix4fv);
	resolve_symbol("glValidateProgram", ::glValidateProgram);
	resolve_symbol("glVertexAttrib1d", ::glVertexAttrib1d);
	resolve_symbol("glVertexAttrib1dv", ::glVertexAttrib1dv);
	resolve_symbol("glVertexAttrib1f", ::glVertexAttrib1f);
	resolve_symbol("glVertexAttrib1fv", ::glVertexAttrib1fv);
	resolve_symbol("glVertexAttrib1s", ::glVertexAttrib1s);
	resolve_symbol("glVertexAttrib1sv", ::glVertexAttrib1sv);
	resolve_symbol("glVertexAttrib2d", ::glVertexAttrib2d);
	resolve_symbol("glVertexAttrib2dv", ::glVertexAttrib2dv);
	resolve_symbol("glVertexAttrib2f", ::glVertexAttrib2f);
	resolve_symbol("glVertexAttrib2fv", ::glVertexAttrib2fv);
	resolve_symbol("glVertexAttrib2s", ::glVertexAttrib2s);
	resolve_symbol("glVertexAttrib2sv", ::glVertexAttrib2sv);
	resolve_symbol("glVertexAttrib3d", ::glVertexAttrib3d);
	resolve_symbol("glVertexAttrib3dv", ::glVertexAttrib3dv);
	resolve_symbol("glVertexAttrib3f", ::glVertexAttrib3f);
	resolve_symbol("glVertexAttrib3fv", ::glVertexAttrib3fv);
	resolve_symbol("glVertexAttrib3s", ::glVertexAttrib3s);
	resolve_symbol("glVertexAttrib3sv", ::glVertexAttrib3sv);
	resolve_symbol("glVertexAttrib4Nbv", ::glVertexAttrib4Nbv);
	resolve_symbol("glVertexAttrib4Niv", ::glVertexAttrib4Niv);
	resolve_symbol("glVertexAttrib4Nsv", ::glVertexAttrib4Nsv);
	resolve_symbol("glVertexAttrib4Nub", ::glVertexAttrib4Nub);
	resolve_symbol("glVertexAttrib4Nubv", ::glVertexAttrib4Nubv);
	resolve_symbol("glVertexAttrib4Nuiv", ::glVertexAttrib4Nuiv);
	resolve_symbol("glVertexAttrib4Nusv", ::glVertexAttrib4Nusv);
	resolve_symbol("glVertexAttrib4bv", ::glVertexAttrib4bv);
	resolve_symbol("glVertexAttrib4d", ::glVertexAttrib4d);
	resolve_symbol("glVertexAttrib4dv", ::glVertexAttrib4dv);
	resolve_symbol("glVertexAttrib4f", ::glVertexAttrib4f);
	resolve_symbol("glVertexAttrib4fv", ::glVertexAttrib4fv);
	resolve_symbol("glVertexAttrib4iv", ::glVertexAttrib4iv);
	resolve_symbol("glVertexAttrib4s", ::glVertexAttrib4s);
	resolve_symbol("glVertexAttrib4sv", ::glVertexAttrib4sv);
	resolve_symbol("glVertexAttrib4ubv", ::glVertexAttrib4ubv);
	resolve_symbol("glVertexAttrib4uiv", ::glVertexAttrib4uiv);
	resolve_symbol("glVertexAttrib4usv", ::glVertexAttrib4usv);
	resolve_symbol("glVertexAttribPointer", ::glVertexAttribPointer);
}

void OglExtensionManagerImpl::resolve_v2_1()
{
	resolve_symbol("glUniformMatrix2x3fv", ::glUniformMatrix2x3fv);
	resolve_symbol("glUniformMatrix3x2fv", ::glUniformMatrix3x2fv);
	resolve_symbol("glUniformMatrix2x4fv", ::glUniformMatrix2x4fv);
	resolve_symbol("glUniformMatrix4x2fv", ::glUniformMatrix4x2fv);
	resolve_symbol("glUniformMatrix3x4fv", ::glUniformMatrix3x4fv);
	resolve_symbol("glUniformMatrix4x3fv", ::glUniformMatrix4x3fv);
}

void OglExtensionManagerImpl::resolve_v3_0()
{
	resolve_symbol("glColorMaski", ::glColorMaski);
	resolve_symbol("glGetBooleani_v", ::glGetBooleani_v);
	resolve_symbol("glGetIntegeri_v", ::glGetIntegeri_v);
	resolve_symbol("glEnablei", ::glEnablei);
	resolve_symbol("glDisablei", ::glDisablei);
	resolve_symbol("glIsEnabledi", ::glIsEnabledi);
	resolve_symbol("glBeginTransformFeedback", ::glBeginTransformFeedback);
	resolve_symbol("glEndTransformFeedback", ::glEndTransformFeedback);
	resolve_symbol("glBindBufferRange", ::glBindBufferRange);
	resolve_symbol("glBindBufferBase", ::glBindBufferBase);
	resolve_symbol("glTransformFeedbackVaryings", ::glTransformFeedbackVaryings);
	resolve_symbol("glGetTransformFeedbackVarying", ::glGetTransformFeedbackVarying);
	resolve_symbol("glClampColor", ::glClampColor);
	resolve_symbol("glBeginConditionalRender", ::glBeginConditionalRender);
	resolve_symbol("glEndConditionalRender", ::glEndConditionalRender);
	resolve_symbol("glVertexAttribIPointer", ::glVertexAttribIPointer);
	resolve_symbol("glGetVertexAttribIiv", ::glGetVertexAttribIiv);
	resolve_symbol("glGetVertexAttribIuiv", ::glGetVertexAttribIuiv);
	resolve_symbol("glVertexAttribI1i", ::glVertexAttribI1i);
	resolve_symbol("glVertexAttribI2i", ::glVertexAttribI2i);
	resolve_symbol("glVertexAttribI3i", ::glVertexAttribI3i);
	resolve_symbol("glVertexAttribI4i", ::glVertexAttribI4i);
	resolve_symbol("glVertexAttribI1ui", ::glVertexAttribI1ui);
	resolve_symbol("glVertexAttribI2ui", ::glVertexAttribI2ui);
	resolve_symbol("glVertexAttribI3ui", ::glVertexAttribI3ui);
	resolve_symbol("glVertexAttribI4ui", ::glVertexAttribI4ui);
	resolve_symbol("glVertexAttribI1iv", ::glVertexAttribI1iv);
	resolve_symbol("glVertexAttribI2iv", ::glVertexAttribI2iv);
	resolve_symbol("glVertexAttribI3iv", ::glVertexAttribI3iv);
	resolve_symbol("glVertexAttribI4iv", ::glVertexAttribI4iv);
	resolve_symbol("glVertexAttribI1uiv", ::glVertexAttribI1uiv);
	resolve_symbol("glVertexAttribI2uiv", ::glVertexAttribI2uiv);
	resolve_symbol("glVertexAttribI3uiv", ::glVertexAttribI3uiv);
	resolve_symbol("glVertexAttribI4uiv", ::glVertexAttribI4uiv);
	resolve_symbol("glVertexAttribI4bv", ::glVertexAttribI4bv);
	resolve_symbol("glVertexAttribI4sv", ::glVertexAttribI4sv);
	resolve_symbol("glVertexAttribI4ubv", ::glVertexAttribI4ubv);
	resolve_symbol("glVertexAttribI4usv", ::glVertexAttribI4usv);
	resolve_symbol("glGetUniformuiv", ::glGetUniformuiv);
	resolve_symbol("glBindFragDataLocation", ::glBindFragDataLocation);
	resolve_symbol("glGetFragDataLocation", ::glGetFragDataLocation);
	resolve_symbol("glUniform1ui", ::glUniform1ui);
	resolve_symbol("glUniform2ui", ::glUniform2ui);
	resolve_symbol("glUniform3ui", ::glUniform3ui);
	resolve_symbol("glUniform4ui", ::glUniform4ui);
	resolve_symbol("glUniform1uiv", ::glUniform1uiv);
	resolve_symbol("glUniform2uiv", ::glUniform2uiv);
	resolve_symbol("glUniform3uiv", ::glUniform3uiv);
	resolve_symbol("glUniform4uiv", ::glUniform4uiv);
	resolve_symbol("glTexParameterIiv", ::glTexParameterIiv);
	resolve_symbol("glTexParameterIuiv", ::glTexParameterIuiv);
	resolve_symbol("glGetTexParameterIiv", ::glGetTexParameterIiv);
	resolve_symbol("glGetTexParameterIuiv", ::glGetTexParameterIuiv);
	resolve_symbol("glClearBufferiv", ::glClearBufferiv);
	resolve_symbol("glClearBufferuiv", ::glClearBufferuiv);
	resolve_symbol("glClearBufferfv", ::glClearBufferfv);
	resolve_symbol("glClearBufferfi", ::glClearBufferfi);
	resolve_symbol("glGetStringi", ::glGetStringi);
}

void OglExtensionManagerImpl::resolve_v3_1()
{
	resolve_symbol("glDrawArraysInstanced", ::glDrawArraysInstanced);
	resolve_symbol("glDrawElementsInstanced", ::glDrawElementsInstanced);
	resolve_symbol("glTexBuffer", ::glTexBuffer);
	resolve_symbol("glPrimitiveRestartIndex", ::glPrimitiveRestartIndex);
}

void OglExtensionManagerImpl::resolve_arb_buffer_storage()
{
	resolve_symbol("glBufferStorage", ::glBufferStorage);
}

void OglExtensionManagerImpl::resolve_arb_color_buffer_float()
{
	resolve_symbol("glClampColorARB", ::glClampColorARB);
}

void OglExtensionManagerImpl::resolve_arb_copy_buffer()
{
	resolve_symbol("glCopyBufferSubData", ::glCopyBufferSubData);
}

void OglExtensionManagerImpl::resolve_arb_direct_state_access()
{
	resolve_symbol("glCreateTransformFeedbacks", ::glCreateTransformFeedbacks);
	resolve_symbol("glTransformFeedbackBufferBase", ::glTransformFeedbackBufferBase);
	resolve_symbol("glTransformFeedbackBufferRange", ::glTransformFeedbackBufferRange);
	resolve_symbol("glGetTransformFeedbackiv", ::glGetTransformFeedbackiv);
	resolve_symbol("glGetTransformFeedbacki_v", ::glGetTransformFeedbacki_v);
	resolve_symbol("glGetTransformFeedbacki64_v", ::glGetTransformFeedbacki64_v);
	resolve_symbol("glCreateBuffers", ::glCreateBuffers);
	resolve_symbol("glNamedBufferStorage", ::glNamedBufferStorage);
	resolve_symbol("glNamedBufferData", ::glNamedBufferData);
	resolve_symbol("glNamedBufferSubData", ::glNamedBufferSubData);
	resolve_symbol("glCopyNamedBufferSubData", ::glCopyNamedBufferSubData);
	resolve_symbol("glClearNamedBufferData", ::glClearNamedBufferData);
	resolve_symbol("glClearNamedBufferSubData", ::glClearNamedBufferSubData);
	resolve_symbol("glMapNamedBuffer", ::glMapNamedBuffer);
	resolve_symbol("glMapNamedBufferRange", ::glMapNamedBufferRange);
	resolve_symbol("glUnmapNamedBuffer", ::glUnmapNamedBuffer);
	resolve_symbol("glFlushMappedNamedBufferRange", ::glFlushMappedNamedBufferRange);
	resolve_symbol("glGetNamedBufferParameteriv", ::glGetNamedBufferParameteriv);
	resolve_symbol("glGetNamedBufferParameteri64v", ::glGetNamedBufferParameteri64v);
	resolve_symbol("glGetNamedBufferPointerv", ::glGetNamedBufferPointerv);
	resolve_symbol("glGetNamedBufferSubData", ::glGetNamedBufferSubData);
	resolve_symbol("glCreateFramebuffers", ::glCreateFramebuffers);
	resolve_symbol("glNamedFramebufferRenderbuffer", ::glNamedFramebufferRenderbuffer);
	resolve_symbol("glNamedFramebufferParameteri", ::glNamedFramebufferParameteri);
	resolve_symbol("glNamedFramebufferTexture", ::glNamedFramebufferTexture);
	resolve_symbol("glNamedFramebufferTextureLayer", ::glNamedFramebufferTextureLayer);
	resolve_symbol("glNamedFramebufferDrawBuffer", ::glNamedFramebufferDrawBuffer);
	resolve_symbol("glNamedFramebufferDrawBuffers", ::glNamedFramebufferDrawBuffers);
	resolve_symbol("glNamedFramebufferReadBuffer", ::glNamedFramebufferReadBuffer);
	resolve_symbol("glInvalidateNamedFramebufferData", ::glInvalidateNamedFramebufferData);
	resolve_symbol("glInvalidateNamedFramebufferSubData", ::glInvalidateNamedFramebufferSubData);
	resolve_symbol("glClearNamedFramebufferiv", ::glClearNamedFramebufferiv);
	resolve_symbol("glClearNamedFramebufferuiv", ::glClearNamedFramebufferuiv);
	resolve_symbol("glClearNamedFramebufferfv", ::glClearNamedFramebufferfv);
	resolve_symbol("glClearNamedFramebufferfi", ::glClearNamedFramebufferfi);
	resolve_symbol("glBlitNamedFramebuffer", ::glBlitNamedFramebuffer);
	resolve_symbol("glCheckNamedFramebufferStatus", ::glCheckNamedFramebufferStatus);
	resolve_symbol("glGetNamedFramebufferParameteriv", ::glGetNamedFramebufferParameteriv);
	resolve_symbol("glGetNamedFramebufferAttachmentParameteriv", ::glGetNamedFramebufferAttachmentParameteriv);
	resolve_symbol("glCreateRenderbuffers", ::glCreateRenderbuffers);
	resolve_symbol("glNamedRenderbufferStorage", ::glNamedRenderbufferStorage);
	resolve_symbol("glNamedRenderbufferStorageMultisample", ::glNamedRenderbufferStorageMultisample);
	resolve_symbol("glGetNamedRenderbufferParameteriv", ::glGetNamedRenderbufferParameteriv);
	resolve_symbol("glCreateTextures", ::glCreateTextures);
	resolve_symbol("glTextureBuffer", ::glTextureBuffer);
	resolve_symbol("glTextureBufferRange", ::glTextureBufferRange);
	resolve_symbol("glTextureStorage1D", ::glTextureStorage1D);
	resolve_symbol("glTextureStorage2D", ::glTextureStorage2D);
	resolve_symbol("glTextureStorage3D", ::glTextureStorage3D);
	resolve_symbol("glTextureStorage2DMultisample", ::glTextureStorage2DMultisample);
	resolve_symbol("glTextureStorage3DMultisample", ::glTextureStorage3DMultisample);
	resolve_symbol("glTextureSubImage1D", ::glTextureSubImage1D);
	resolve_symbol("glTextureSubImage2D", ::glTextureSubImage2D);
	resolve_symbol("glTextureSubImage3D", ::glTextureSubImage3D);
	resolve_symbol("glCompressedTextureSubImage1D", ::glCompressedTextureSubImage1D);
	resolve_symbol("glCompressedTextureSubImage2D", ::glCompressedTextureSubImage2D);
	resolve_symbol("glCompressedTextureSubImage3D", ::glCompressedTextureSubImage3D);
	resolve_symbol("glCopyTextureSubImage1D", ::glCopyTextureSubImage1D);
	resolve_symbol("glCopyTextureSubImage2D", ::glCopyTextureSubImage2D);
	resolve_symbol("glCopyTextureSubImage3D", ::glCopyTextureSubImage3D);
	resolve_symbol("glTextureParameterf", ::glTextureParameterf);
	resolve_symbol("glTextureParameterfv", ::glTextureParameterfv);
	resolve_symbol("glTextureParameteri", ::glTextureParameteri);
	resolve_symbol("glTextureParameterIiv", ::glTextureParameterIiv);
	resolve_symbol("glTextureParameterIuiv", ::glTextureParameterIuiv);
	resolve_symbol("glTextureParameteriv", ::glTextureParameteriv);
	resolve_symbol("glGenerateTextureMipmap", ::glGenerateTextureMipmap);
	resolve_symbol("glBindTextureUnit", ::glBindTextureUnit);
	resolve_symbol("glGetTextureImage", ::glGetTextureImage);
	resolve_symbol("glGetCompressedTextureImage", ::glGetCompressedTextureImage);
	resolve_symbol("glGetTextureLevelParameterfv", ::glGetTextureLevelParameterfv);
	resolve_symbol("glGetTextureLevelParameteriv", ::glGetTextureLevelParameteriv);
	resolve_symbol("glGetTextureParameterfv", ::glGetTextureParameterfv);
	resolve_symbol("glGetTextureParameterIiv", ::glGetTextureParameterIiv);
	resolve_symbol("glGetTextureParameterIuiv", ::glGetTextureParameterIuiv);
	resolve_symbol("glGetTextureParameteriv", ::glGetTextureParameteriv);
	resolve_symbol("glCreateVertexArrays", ::glCreateVertexArrays);
	resolve_symbol("glDisableVertexArrayAttrib", ::glDisableVertexArrayAttrib);
	resolve_symbol("glEnableVertexArrayAttrib", ::glEnableVertexArrayAttrib);
	resolve_symbol("glVertexArrayElementBuffer", ::glVertexArrayElementBuffer);
	resolve_symbol("glVertexArrayVertexBuffer", ::glVertexArrayVertexBuffer);
	resolve_symbol("glVertexArrayVertexBuffers", ::glVertexArrayVertexBuffers);
	resolve_symbol("glVertexArrayAttribBinding", ::glVertexArrayAttribBinding);
	resolve_symbol("glVertexArrayAttribFormat", ::glVertexArrayAttribFormat);
	resolve_symbol("glVertexArrayAttribIFormat", ::glVertexArrayAttribIFormat);
	resolve_symbol("glVertexArrayAttribLFormat", ::glVertexArrayAttribLFormat);
	resolve_symbol("glVertexArrayBindingDivisor", ::glVertexArrayBindingDivisor);
	resolve_symbol("glGetVertexArrayiv", ::glGetVertexArrayiv);
	resolve_symbol("glGetVertexArrayIndexediv", ::glGetVertexArrayIndexediv);
	resolve_symbol("glGetVertexArrayIndexed64iv", ::glGetVertexArrayIndexed64iv);
	resolve_symbol("glCreateSamplers", ::glCreateSamplers);
	resolve_symbol("glCreateProgramPipelines", ::glCreateProgramPipelines);
	resolve_symbol("glCreateQueries", ::glCreateQueries);
	resolve_symbol("glGetQueryBufferObjecti64v", ::glGetQueryBufferObjecti64v);
	resolve_symbol("glGetQueryBufferObjectiv", ::glGetQueryBufferObjectiv);
	resolve_symbol("glGetQueryBufferObjectui64v", ::glGetQueryBufferObjectui64v);
	resolve_symbol("glGetQueryBufferObjectuiv", ::glGetQueryBufferObjectuiv);
}

void OglExtensionManagerImpl::resolve_arb_framebuffer_object()
{
	resolve_symbol("glIsRenderbuffer", ::glIsRenderbuffer);
	resolve_symbol("glBindRenderbuffer", ::glBindRenderbuffer);
	resolve_symbol("glDeleteRenderbuffers", ::glDeleteRenderbuffers);
	resolve_symbol("glGenRenderbuffers", ::glGenRenderbuffers);
	resolve_symbol("glRenderbufferStorage", ::glRenderbufferStorage);
	resolve_symbol("glGetRenderbufferParameteriv", ::glGetRenderbufferParameteriv);
	resolve_symbol("glIsFramebuffer", ::glIsFramebuffer);
	resolve_symbol("glBindFramebuffer", ::glBindFramebuffer);
	resolve_symbol("glDeleteFramebuffers", ::glDeleteFramebuffers);
	resolve_symbol("glGenFramebuffers", ::glGenFramebuffers);
	resolve_symbol("glCheckFramebufferStatus", ::glCheckFramebufferStatus);
	resolve_symbol("glFramebufferTexture1D", ::glFramebufferTexture1D);
	resolve_symbol("glFramebufferTexture2D", ::glFramebufferTexture2D);
	resolve_symbol("glFramebufferTexture3D", ::glFramebufferTexture3D);
	resolve_symbol("glFramebufferRenderbuffer", ::glFramebufferRenderbuffer);
	resolve_symbol("glGetFramebufferAttachmentParameteriv", ::glGetFramebufferAttachmentParameteriv);
	resolve_symbol("glGenerateMipmap", ::glGenerateMipmap);
	resolve_symbol("glBlitFramebuffer", ::glBlitFramebuffer);
	resolve_symbol("glRenderbufferStorageMultisample", ::glRenderbufferStorageMultisample);
	resolve_symbol("glFramebufferTextureLayer", ::glFramebufferTextureLayer);
}

void OglExtensionManagerImpl::resolve_arb_map_buffer_range()
{
	resolve_symbol("glMapBufferRange", ::glMapBufferRange);
	resolve_symbol("glFlushMappedBufferRange", ::glFlushMappedBufferRange);
}

void OglExtensionManagerImpl::resolve_arb_sampler_objects()
{
	resolve_symbol("glGenSamplers", ::glGenSamplers);
	resolve_symbol("glDeleteSamplers", ::glDeleteSamplers);
	resolve_symbol("glIsSampler", ::glIsSampler);
	resolve_symbol("glBindSampler", ::glBindSampler);
	resolve_symbol("glSamplerParameteri", ::glSamplerParameteri);
	resolve_symbol("glSamplerParameteriv", ::glSamplerParameteriv);
	resolve_symbol("glSamplerParameterf", ::glSamplerParameterf);
	resolve_symbol("glSamplerParameterfv", ::glSamplerParameterfv);
	resolve_symbol("glSamplerParameterIiv", ::glSamplerParameterIiv);
	resolve_symbol("glSamplerParameterIuiv", ::glSamplerParameterIuiv);
	resolve_symbol("glGetSamplerParameteriv", ::glGetSamplerParameteriv);
	resolve_symbol("glGetSamplerParameterIiv", ::glGetSamplerParameterIiv);
	resolve_symbol("glGetSamplerParameterfv", ::glGetSamplerParameterfv);
	resolve_symbol("glGetSamplerParameterIuiv", ::glGetSamplerParameterIuiv);
}

void OglExtensionManagerImpl::resolve_arb_separate_shader_objects()
{
	resolve_symbol("glUseProgramStages", ::glUseProgramStages);
	resolve_symbol("glActiveShaderProgram", ::glActiveShaderProgram);
	resolve_symbol("glCreateShaderProgramv", ::glCreateShaderProgramv);
	resolve_symbol("glBindProgramPipeline", ::glBindProgramPipeline);
	resolve_symbol("glDeleteProgramPipelines", ::glDeleteProgramPipelines);
	resolve_symbol("glGenProgramPipelines", ::glGenProgramPipelines);
	resolve_symbol("glIsProgramPipeline", ::glIsProgramPipeline);
	resolve_symbol("glGetProgramPipelineiv", ::glGetProgramPipelineiv);
	resolve_symbol("glProgramParameteri", ::glProgramParameteri);
	resolve_symbol("glProgramUniform1i", ::glProgramUniform1i);
	resolve_symbol("glProgramUniform1iv", ::glProgramUniform1iv);
	resolve_symbol("glProgramUniform1f", ::glProgramUniform1f);
	resolve_symbol("glProgramUniform1fv", ::glProgramUniform1fv);
	resolve_symbol("glProgramUniform1d", ::glProgramUniform1d);
	resolve_symbol("glProgramUniform1dv", ::glProgramUniform1dv);
	resolve_symbol("glProgramUniform1ui", ::glProgramUniform1ui);
	resolve_symbol("glProgramUniform1uiv", ::glProgramUniform1uiv);
	resolve_symbol("glProgramUniform2i", ::glProgramUniform2i);
	resolve_symbol("glProgramUniform2iv", ::glProgramUniform2iv);
	resolve_symbol("glProgramUniform2f", ::glProgramUniform2f);
	resolve_symbol("glProgramUniform2fv", ::glProgramUniform2fv);
	resolve_symbol("glProgramUniform2d", ::glProgramUniform2d);
	resolve_symbol("glProgramUniform2dv", ::glProgramUniform2dv);
	resolve_symbol("glProgramUniform2ui", ::glProgramUniform2ui);
	resolve_symbol("glProgramUniform2uiv", ::glProgramUniform2uiv);
	resolve_symbol("glProgramUniform3i", ::glProgramUniform3i);
	resolve_symbol("glProgramUniform3iv", ::glProgramUniform3iv);
	resolve_symbol("glProgramUniform3f", ::glProgramUniform3f);
	resolve_symbol("glProgramUniform3fv", ::glProgramUniform3fv);
	resolve_symbol("glProgramUniform3d", ::glProgramUniform3d);
	resolve_symbol("glProgramUniform3dv", ::glProgramUniform3dv);
	resolve_symbol("glProgramUniform3ui", ::glProgramUniform3ui);
	resolve_symbol("glProgramUniform3uiv", ::glProgramUniform3uiv);
	resolve_symbol("glProgramUniform4i", ::glProgramUniform4i);
	resolve_symbol("glProgramUniform4iv", ::glProgramUniform4iv);
	resolve_symbol("glProgramUniform4f", ::glProgramUniform4f);
	resolve_symbol("glProgramUniform4fv", ::glProgramUniform4fv);
	resolve_symbol("glProgramUniform4d", ::glProgramUniform4d);
	resolve_symbol("glProgramUniform4dv", ::glProgramUniform4dv);
	resolve_symbol("glProgramUniform4ui", ::glProgramUniform4ui);
	resolve_symbol("glProgramUniform4uiv", ::glProgramUniform4uiv);
	resolve_symbol("glProgramUniformMatrix2fv", ::glProgramUniformMatrix2fv);
	resolve_symbol("glProgramUniformMatrix3fv", ::glProgramUniformMatrix3fv);
	resolve_symbol("glProgramUniformMatrix4fv", ::glProgramUniformMatrix4fv);
	resolve_symbol("glProgramUniformMatrix2dv", ::glProgramUniformMatrix2dv);
	resolve_symbol("glProgramUniformMatrix3dv", ::glProgramUniformMatrix3dv);
	resolve_symbol("glProgramUniformMatrix4dv", ::glProgramUniformMatrix4dv);
	resolve_symbol("glProgramUniformMatrix2x3fv", ::glProgramUniformMatrix2x3fv);
	resolve_symbol("glProgramUniformMatrix3x2fv", ::glProgramUniformMatrix3x2fv);
	resolve_symbol("glProgramUniformMatrix2x4fv", ::glProgramUniformMatrix2x4fv);
	resolve_symbol("glProgramUniformMatrix4x2fv", ::glProgramUniformMatrix4x2fv);
	resolve_symbol("glProgramUniformMatrix3x4fv", ::glProgramUniformMatrix3x4fv);
	resolve_symbol("glProgramUniformMatrix4x3fv", ::glProgramUniformMatrix4x3fv);
	resolve_symbol("glProgramUniformMatrix2x3dv", ::glProgramUniformMatrix2x3dv);
	resolve_symbol("glProgramUniformMatrix3x2dv", ::glProgramUniformMatrix3x2dv);
	resolve_symbol("glProgramUniformMatrix2x4dv", ::glProgramUniformMatrix2x4dv);
	resolve_symbol("glProgramUniformMatrix4x2dv", ::glProgramUniformMatrix4x2dv);
	resolve_symbol("glProgramUniformMatrix3x4dv", ::glProgramUniformMatrix3x4dv);
	resolve_symbol("glProgramUniformMatrix4x3dv", ::glProgramUniformMatrix4x3dv);
	resolve_symbol("glValidateProgramPipeline", ::glValidateProgramPipeline);
	resolve_symbol("glGetProgramPipelineInfoLog", ::glGetProgramPipelineInfoLog);
}

void OglExtensionManagerImpl::resolve_arb_uniform_buffer_object()
{
	resolve_symbol("glGetUniformIndices", ::glGetUniformIndices);
	resolve_symbol("glGetActiveUniformsiv", ::glGetActiveUniformsiv);
	resolve_symbol("glGetActiveUniformName", ::glGetActiveUniformName);
	resolve_symbol("glGetUniformBlockIndex", ::glGetUniformBlockIndex);
	resolve_symbol("glGetActiveUniformBlockiv", ::glGetActiveUniformBlockiv);
	resolve_symbol("glGetActiveUniformBlockName", ::glGetActiveUniformBlockName);
	resolve_symbol("glUniformBlockBinding", ::glUniformBlockBinding);
	resolve_symbol("glBindBufferRange", ::glBindBufferRange);
	resolve_symbol("glBindBufferBase", ::glBindBufferBase);
	resolve_symbol("glGetIntegeri_v", ::glGetIntegeri_v);
}

void OglExtensionManagerImpl::resolve_arb_vertex_array_object()
{
	resolve_symbol("glBindVertexArray", ::glBindVertexArray);
	resolve_symbol("glDeleteVertexArrays", ::glDeleteVertexArrays);
	resolve_symbol("glGenVertexArrays", ::glGenVertexArrays);
	resolve_symbol("glIsVertexArray", ::glIsVertexArray);
}

void OglExtensionManagerImpl::resolve_arb_vertex_buffer_object()
{
	resolve_symbol("glBindBufferARB", ::glBindBufferARB);
	resolve_symbol("glDeleteBuffersARB", ::glDeleteBuffersARB);
	resolve_symbol("glGenBuffersARB", ::glGenBuffersARB);
	resolve_symbol("glIsBufferARB", ::glIsBufferARB);
	resolve_symbol("glBufferDataARB", ::glBufferDataARB);
	resolve_symbol("glBufferSubDataARB", ::glBufferSubDataARB);
	resolve_symbol("glGetBufferSubDataARB", ::glGetBufferSubDataARB);
	resolve_symbol("glMapBufferARB", ::glMapBufferARB);
	resolve_symbol("glUnmapBufferARB", ::glUnmapBufferARB);
	resolve_symbol("glGetBufferParameterivARB", ::glGetBufferParameterivARB);
	resolve_symbol("glGetBufferPointervARB", ::glGetBufferPointervARB);
}

void OglExtensionManagerImpl::resolve_ext_framebuffer_blit()
{
	resolve_symbol("glBlitFramebufferEXT", ::glBlitFramebufferEXT);
}

void OglExtensionManagerImpl::resolve_ext_framebuffer_multisample()
{
	resolve_symbol("glRenderbufferStorageMultisampleEXT", ::glRenderbufferStorageMultisampleEXT);
}

void OglExtensionManagerImpl::resolve_ext_framebuffer_object()
{
	resolve_symbol("glIsRenderbufferEXT", ::glIsRenderbufferEXT);
	resolve_symbol("glBindRenderbufferEXT", ::glBindRenderbufferEXT);
	resolve_symbol("glDeleteRenderbuffersEXT", ::glDeleteRenderbuffersEXT);
	resolve_symbol("glGenRenderbuffersEXT", ::glGenRenderbuffersEXT);
	resolve_symbol("glRenderbufferStorageEXT", ::glRenderbufferStorageEXT);
	resolve_symbol("glGetRenderbufferParameterivEXT", ::glGetRenderbufferParameterivEXT);
	resolve_symbol("glIsFramebufferEXT", ::glIsFramebufferEXT);
	resolve_symbol("glBindFramebufferEXT", ::glBindFramebufferEXT);
	resolve_symbol("glDeleteFramebuffersEXT", ::glDeleteFramebuffersEXT);
	resolve_symbol("glGenFramebuffersEXT", ::glGenFramebuffersEXT);
	resolve_symbol("glCheckFramebufferStatusEXT", ::glCheckFramebufferStatusEXT);
	resolve_symbol("glFramebufferTexture1DEXT", ::glFramebufferTexture1DEXT);
	resolve_symbol("glFramebufferTexture2DEXT", ::glFramebufferTexture2DEXT);
	resolve_symbol("glFramebufferTexture3DEXT", ::glFramebufferTexture3DEXT);
	resolve_symbol("glFramebufferRenderbufferEXT", ::glFramebufferRenderbufferEXT);
	resolve_symbol("glGetFramebufferAttachmentParameterivEXT", ::glGetFramebufferAttachmentParameterivEXT);
	resolve_symbol("glGenerateMipmapEXT", ::glGenerateMipmapEXT);
}

//
// OglExtensionManagerImpl
// ==========================================================================


// ==========================================================================
// OglExtensionManager
//

OglExtensionManager::OglExtensionManager() = default;

OglExtensionManager::~OglExtensionManager() = default;

//
// OglExtensionManager
// ==========================================================================


// ==========================================================================
// OglExtensionManagerFactory
//

OglExtensionManagerUPtr OglExtensionManagerFactory::create()
{
	return OglExtensionManagerImplUPtr{new OglExtensionManagerImpl{}};
}

//
// OglExtensionManagerFactory
// ==========================================================================


} // detail
} // bstone
