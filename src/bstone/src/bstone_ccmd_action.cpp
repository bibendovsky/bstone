/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_ccmd_action.h"

namespace bstone {

CCmdAction::CCmdAction() noexcept = default;
CCmdAction::~CCmdAction() = default;

void CCmdAction::invoke(CCmdActionArgs args)
try {
	do_invoke(args);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void CCmdAction::operator()(CCmdActionArgs args)
try {
	invoke(args);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone
