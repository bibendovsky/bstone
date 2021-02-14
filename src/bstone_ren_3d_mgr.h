/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
// 3D renderer manager.
//


#ifndef BSTONE_REN_3D_MGR_INCLUDED
#define BSTONE_REN_3D_MGR_INCLUDED


#include "bstone_ren_3d.h"


namespace bstone
{


// ==========================================================================
// Ren3dMgr
//

class Ren3dMgr
{
protected:
	Ren3dMgr() = default;


public:
	virtual ~Ren3dMgr() = default;


	virtual Ren3dPtr renderer_initialize(
		const Ren3dCreateParam& param) = 0;
}; // Ren3dMgr

using Ren3dMgrUPtr = std::unique_ptr<Ren3dMgr>;

//
// Ren3dMgr
// ==========================================================================


// ==========================================================================
// Ren3dMgrFactory
//

struct Ren3dMgrFactory
{
	static Ren3dMgrUPtr create();
}; // Ren3dMgrFactory

//
// Ren3dMgrFactory
// ==========================================================================


} // bstone


#endif // !BSTONE_REN_3D_MGR_INCLUDED
