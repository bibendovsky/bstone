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
// OpenGL renderer.
//
// !!! Internal usage only !!!
//


#ifndef BSTONE_DETAIL_OGL_RENDERER_INCLUDED
#define BSTONE_DETAIL_OGL_RENDERER_INCLUDED


#include "bstone_renderer.h"


namespace bstone
{
namespace detail
{


class OglXRenderer :
	public Renderer
{
public:
}; // OglXRenderer

using OglXRendererUPtr = std::unique_ptr<OglXRenderer>;


class OglRenderer :
	public OglXRenderer
{
public:
	OglRenderer();

	OglRenderer(
		OglRenderer&& rhs);

	~OglRenderer() override;


	const std::string& get_error_message() const override;


	RendererKind get_kind() const override;

	const std::string& get_name() const override;

	const std::string& get_description() const override;


	bool probe(
		const RendererPath renderer_path) override;

	const RendererProbe& probe_get() const override;


	bool is_initialized() const override;

	bool initialize(
		const RendererInitializeParam& param) override;

	void uninitialize() override;


	RendererPath get_path() const override;


	const RendererDeviceFeatures& device_get_features() const override;

	bool device_is_lost() const override;

	bool device_is_ready_to_reset() const override;

	void device_reset() override;


	void window_show(
		const bool is_visible) override;


	void color_buffer_set_clear_color(
		const RendererColor32& color) override;

	void palette_update(
		const RendererPalette& palette) override;

	void clear_buffers() override;

	void present() override;


	RendererIndexBufferPtr index_buffer_create(
		const RendererIndexBufferCreateParam& param) override;

	void index_buffer_destroy(
		RendererIndexBufferPtr index_buffer) override;


	RendererVertexBufferPtr vertex_buffer_create(
		const RendererVertexBufferCreateParam& param) override;

	void vertex_buffer_destroy(
		RendererVertexBufferPtr vertex_buffer) override;


	RendererTexture2dPtr texture_2d_create(
		const RendererTexture2dCreateParam& param) override;

	void texture_2d_destroy(
		RendererTexture2dPtr texture_2d) override;


	RendererSamplerPtr sampler_create(
		const RendererSamplerCreateParam& param) override;

	void sampler_destroy(
		RendererSamplerPtr sampler) override;


	RendererVertexInputPtr vertex_input_create(
		const RendererVertexInputCreateParam& param) override;

	void vertex_input_destroy(
		RendererVertexInputPtr vertex_input) override;


	void execute_commands(
		const RendererCommandManagerPtr command_manager) override;


private:
	bool is_initialized_;
	std::string error_message_;

	RendererProbe renderer_probe_;
	RendererPath renderer_path_;
	OglXRendererUPtr renderer_;


	template<typename TRenderer>
	bool probe_ogl_x(
		const RendererPath renderer_path)
	{
		auto ogl_x_renderer = TRenderer{};

		if (!ogl_x_renderer.probe(renderer_path))
		{
			renderer_probe_.path_ = RendererPath::none;

			return false;
		}

		renderer_probe_.path_ = renderer_path;
		renderer_probe_.device_features_ = ogl_x_renderer.device_get_features();

		return true;
	}

	template<typename TRenderer>
	bool initialize_ogl_x(
		const RendererInitializeParam param)
	{
		auto renderer = OglXRendererUPtr{new TRenderer{}};

		if (!renderer->initialize(param))
		{
			error_message_ = renderer->get_error_message();

			return false;
		}

		is_initialized_ = true;
		renderer_ = std::move(renderer);

		return true;
	}

	void uninitialize_internal(
		const bool is_dtor = false);
}; // OglRenderer


} // detail
} // bstone


#endif // !BSTONE_DETAIL_OGL_RENDERER_INCLUDED
