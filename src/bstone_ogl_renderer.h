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


#ifndef BSTONE_OGL_RENDERER_INCLUDED
#define BSTONE_OGL_RENDERER_INCLUDED


#include "bstone_renderer.h"


namespace bstone
{


class OglRenderer :
	public Renderer
{
public:
	OglRenderer() = default;

	~OglRenderer() override = default;


	const std::string& get_error_message() const override;


	RendererKind get_kind() const override;

	const std::string& get_name() const override;

	const std::string& get_description() const override;


	bool probe(
		const RendererPath renderer_path,
		RendererPath& selected_renderer_path) override;


	bool is_initialized() const override;

	bool initialize(
		const RendererPath renderer_path) override;

	void uninitialize() override;


	RendererPath get_path() const override;
}; // OglRenderer


} // bstone


#endif // !BSTONE_OGL_RENDERER_INCLUDED
