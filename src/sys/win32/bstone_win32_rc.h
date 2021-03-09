#define VS_VERSION_INFO 1

#ifndef VS_FFI_FILEFLAGSMASK
#define VS_FFI_FILEFLAGSMASK 0x0000003FL
#endif // !VS_FFI_FILEFLAGSMASK

#ifdef _DEBUG
#define VER_DBG 1
#else // _DEBUG
#define VER_DBG 0
#endif // _DEBUG

#ifndef VOS_NT
#define VOS_NT 0x00040000L
#endif // !VOS_NT

#ifndef VFT_DRV
#define VFT_DRV 0x00000003L
#endif // !VFT_DRV

#ifndef VFT2_DRV_SYSTEM
#define VFT2_DRV_SYSTEM 0x00000007L
#endif // !VFT2_DRV_SYSTEM

#ifndef RT_MANIFEST
#define RT_MANIFEST 24
#endif // !RT_MANIFEST
