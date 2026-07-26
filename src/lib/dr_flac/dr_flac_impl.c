/*
Implementation translation unit for dr_flac.

dr_flac.h is a normal header unless DR_FLAC_IMPLEMENTATION is defined in exactly
one translation unit; this is that unit. DR_FLAC_NO_STDIO drops the FILE*-based
helpers bstone does not use (it decodes from an in-memory buffer), and
DR_FLAC_NO_OGG drops Ogg-encapsulated FLAC, which bstone does not support.
*/

#define DR_FLAC_IMPLEMENTATION
#define DR_FLAC_NO_STDIO
#define DR_FLAC_NO_OGG

#include "dr_flac.h"
