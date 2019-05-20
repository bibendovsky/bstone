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
// OpenGL extension registry.
//
// !!! Internal usage only !!!
//


#ifndef BSTONE_DETAIL_OGL_EXTENSION_REGISTRY_INCLUDED
#define BSTONE_DETAIL_OGL_EXTENSION_REGISTRY_INCLUDED


#include <memory>


namespace bstone
{
namespace detail
{


enum class OglExtensionId
{
	none,

	v1_0,
	v1_1,
}; // OglExtensionId


class OglExtensionRegistry
{
protected:
	OglExtensionRegistry();


public:
	virtual ~OglExtensionRegistry();


	virtual void extension_probe(
		const OglExtensionId extension_id) = 0;


	virtual bool has_extension(
		const OglExtensionId extension_id) const = 0;
}; // OglExtensionRegistry

using OglExtensionRegistryPtr = OglExtensionRegistry*;
using OglExtensionRegistryUPtr = std::unique_ptr<OglExtensionRegistry>;


struct OglExtensionRegistryFactory
{
	static OglExtensionRegistryUPtr create();
}; // OglExtensionRegistryFactory


} // detail
} // bstone


#endif // !BSTONE_DETAIL_OGL_EXTENSION_REGISTRY_INCLUDED
