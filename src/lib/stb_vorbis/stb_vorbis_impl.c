/*
Implementation translation unit for stb_vorbis.

stb_vorbis.c is both the public interface and the implementation; this file
compiles the implementation with the configuration bstone needs. Consumers pull
in the declarations through stb_vorbis.h, which defines the same macros so the
two agree on the exposed API surface.
*/

#define STB_VORBIS_NO_STDIO

#include "stb_vorbis.c"
