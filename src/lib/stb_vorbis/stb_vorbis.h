/*
Public header shim for stb_vorbis.

Upstream ships the interface inside stb_vorbis.c and exposes only the
declarations when STB_VORBIS_HEADER_ONLY is defined. The configuration macros
here must match stb_vorbis_impl.c so the declared API matches the compiled one.
*/

#ifndef BSTONE_STB_VORBIS_INCLUDED
#define BSTONE_STB_VORBIS_INCLUDED

#define STB_VORBIS_NO_STDIO
#define STB_VORBIS_NO_PUSHDATA_API
#define STB_VORBIS_HEADER_ONLY

#include "stb_vorbis.c"

#endif // BSTONE_STB_VORBIS_INCLUDED
