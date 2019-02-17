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
// Renderer manager.
//


#ifndef BSTONE_RENDERER_MANAGER_INCLUDED
#define BSTONE_RENDERER_MANAGER_INCLUDED


#include "bstone_renderer.h"


namespace bstone
{


class RendererManager
{
protected:
	RendererManager() = default;


public:
	virtual ~RendererManager() = default;


	virtual bool is_initialized() const = 0;

	virtual const std::string& get_error_message() const = 0;

	virtual bool initialize() = 0;

	virtual void uninitialize() = 0;


	virtual bool renderer_probe(
		const RendererPath& renderer_path) = 0;

	virtual RendererPath renderer_get_probe_path() const = 0;

	virtual RendererPtr renderer_initialize(
		const RendererInitializeParam& param) = 0;
}; // RendererManager

using RendererManagerUPtr = std::unique_ptr<RendererManager>;


class RendererManagerFactory
{
public:
	static RendererManagerUPtr create();
}; // RendererManagerFactory


} // bstone


#endif // !BSTONE_RENDERER_MANAGER_INCLUDED
