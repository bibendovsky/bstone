/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// SDL types.
//


#ifndef BSTONE_SDL_TYPES_INCLUDED
#define BSTONE_SDL_TYPES_INCLUDED


#include <memory>

#include "SDL.h"

#include "bstone_unique_resource.h"


namespace bstone
{


template<typename TObject, void (*TDeleter)(TObject*)>
struct SdlDeleter
{
	void operator()(
		TObject* object) const noexcept
	{
		TDeleter(object);
	}
};

template<typename TObject, void (*TDeleter)(TObject*)>
using SdlUPtr = std::unique_ptr<TObject, SdlDeleter<TObject, TDeleter>>;


using SdlWindowPtr = SDL_Window*;
using SdlWindowUPtr = SdlUPtr<SDL_Window, SDL_DestroyWindow>;

using SdlGlContextPtr = SDL_GLContext;
using SdlGlContextUPtr = SdlUPtr<void, SDL_GL_DeleteContext>;

using SdlSurfacePtr = SDL_Surface*;
using SdlSurfaceUPtr = SdlUPtr<SDL_Surface, SDL_FreeSurface>;

using SdlPixelFormatPtr = SDL_PixelFormat*;
using SdlPixelFormatUPtr = SdlUPtr<SDL_PixelFormat, SDL_FreeFormat>;

using SdlTexturePtr = SDL_Texture*;
using SdlTextureUPtr = SdlUPtr<SDL_Texture, SDL_DestroyTexture>;

using SdlRendererPtr = SDL_Renderer*;
using SdlRendererUPtr = SdlUPtr<SDL_Renderer, SDL_DestroyRenderer>;

using SdlRwOpsUPtr = SdlUPtr<SDL_RWops, SDL_FreeRW>;

using SdlAudioDevice = UniqueResource<SDL_AudioDeviceID, SDL_CloseAudioDevice>;


} //  bstone


#endif // !BSTONE_SDL_TYPES_INCLUDED
