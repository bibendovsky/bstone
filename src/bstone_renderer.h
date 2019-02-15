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
#include "bstone_vecn.h"


namespace bstone
{


using RendererObjectId = void*;

constexpr auto RendererNullObjectId = RendererObjectId{};


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


class RendererColor32
{
public:
	std::uint8_t r_;
	std::uint8_t g_;
	std::uint8_t b_;
	std::uint8_t a_;


	constexpr RendererColor32()
		:
		r_{},
		g_{},
		b_{},
		a_{}
	{
	}

	constexpr RendererColor32(
		const std::uint8_t r,
		const std::uint8_t g,
		const std::uint8_t b,
		const std::uint8_t a)
		:
		r_{},
		g_{},
		b_{},
		a_{}
	{
	}
}; // RendererColor32

using RendererPalette = std::array<RendererColor32, 256>;

class RendererVertex
{
public:
	Vec3F xyz_;
	RendererColor32 rgba_;
	Vec2F uv_;
}; // RendererVertex

class RendererTextureCreateParam
{
public:
	int width_;
	int height_;

	const std::uint8_t* indexed_data_;
}; // RendererTextureCreateParam

class RendererTextureUpdateParam
{
public:
	const std::uint8_t* indexed_data_;
}; // RendererTextureUpdateParam


class Renderer
{
protected:
	Renderer() = default;

	virtual ~Renderer() = default;


public:
	static constexpr auto RendererNullObjectId = RendererObjectId{};


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


	virtual RendererObjectId vertex_buffer_create(
		const int vertex_count) = 0;

	virtual void vertex_buffer_destroy(
		RendererObjectId id) = 0;

	virtual void vertex_buffer_update(
		RendererObjectId id,
		const int offset,
		const int count,
		const RendererVertex* const vertices) = 0;


	virtual RendererObjectId texture_2d_create(
		const RendererTextureCreateParam& param) = 0;

	virtual void texture_2d_destroy(
		RendererObjectId texture_id) = 0;

	virtual void texture_2d_update(
		RendererObjectId texture_id,
		const RendererTextureUpdateParam& param) = 0;
}; // Renderer

using RendererPtr = Renderer*;


} // bstone


#endif // !BSTONE_RENDERER_INCLUDED
