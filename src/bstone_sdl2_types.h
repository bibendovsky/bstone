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
// SDL2 types.
//


#ifndef BSTONE_SDL2_TYPES_INCLUDED
#define BSTONE_SDL2_TYPES_INCLUDED


#include <memory>

#include "SDL_render.h"
#include "SDL_surface.h"
#include "SDL_video.h"


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


} //  bstone


#endif // !BSTONE_SDL2_TYPES_INCLUDED
