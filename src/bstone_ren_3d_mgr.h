/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
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
public:
	Ren3dMgr() noexcept = default;

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
