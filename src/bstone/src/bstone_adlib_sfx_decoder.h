/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

//
// AdLib sound effects decoder.
//

#ifndef BSTONE_ADLIB_SFX_DECODER_INCLUDED
#define BSTONE_ADLIB_SFX_DECODER_INCLUDED

#include "bstone_audio_decoder.h"

namespace bstone
{

AudioDecoderUPtr make_adlib_sfx_audio_decoder(Opl3Type opl3_type);

} // bstone

#endif // !BSTONE_ADLIB_SFX_DECODER_INCLUDED
