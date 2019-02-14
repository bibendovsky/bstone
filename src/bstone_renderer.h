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
// Renderer interface.
//


#ifndef BSTONE_RENDERER_INCLUDED
#define BSTONE_RENDERER_INCLUDED


#include <string>
#include "bstone_color32.h"
#include "bstone_vecn.h"


namespace bstone
{


enum class RendererKind
{
	none,
	opengl,
}; // RendererKind

enum class RendererPath
{
	none,

	autodetect,

	ogl_1_x,
}; // RendererPath


class RendererInitializeWindowParam
{
public:
	bool is_visible_;
	bool is_fullscreen_desktop_;

	int width_;
	int height_;

	std::string title_utf8_;
}; // RendererInitializeWindowParam

class RendererInitializeParam
{
public:
	RendererPath renderer_path_;
	RendererInitializeWindowParam window_;
}; // RendererInitializeParam

class RendererVertex
{
public:
	static constexpr auto class_size = 24;


	Vec3F xyz_;
	Color32 rgba_;
	Vec2F uv_;
}; // RendererVertex

static_assert(RendererVertex::class_size == sizeof(RendererVertex), "Class size mismatch.");


class Renderer
{
protected:
	Renderer() = default;

	virtual ~Renderer() = default;


public:
	using ObjectId = void*;

	static constexpr auto NullObjectId = ObjectId{};


	virtual const std::string& get_error_message() const = 0;


	virtual RendererKind get_kind() const = 0;

	virtual const std::string& get_name() const = 0;

	virtual const std::string& get_description() const = 0;


	virtual bool probe(
		const RendererPath renderer_path) = 0;

	virtual RendererPath get_probe_path() const = 0;


	virtual bool is_initialized() const = 0;

	virtual bool initialize(
		const RendererInitializeParam& param) = 0;

	virtual void uninitialize() = 0;


	virtual RendererPath get_path() const = 0;


	virtual void set_2d_projection_matrix(
		const int width,
		const int height) = 0;


	virtual ObjectId vertex_buffer_create(
		const int vertex_count) = 0;

	virtual void vertex_buffer_destroy(
		ObjectId id) = 0;

	virtual void vertex_buffer_update(
		ObjectId id,
		const int offset,
		const int count,
		const RendererVertex* const vertices) = 0;
}; // Renderer

using RendererPtr = Renderer*;


} // bstone


#endif // !BSTONE_RENDERER_INCLUDED
