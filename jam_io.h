

//
// UNIT : JAM_IO.h
//
// FUNCTION : General defines, prototypes, typedefs used in all the
//				  supported compression techniques used in JAMPAK Ver x.x
//
//


#include "SDL.h"


//=========================================================================
//
// 								FILE CHUNK IDs
//
// NOTE: The only reason for changing from COMP to CMP1 and having multi
//			comp header structs is for downward compatablity.
//

#define COMP					("COMP")		// Comp type is ct_LZW ALWAYS!
#define JAMP					("JAMP")		// New Version different from SOFTDISK.


//
// 	COMPRESSION TYPES
//
typedef enum ct_TYPES
{
		ct_NONE = 0,						// No compression - Just data..Rarely used!
		ct_LZW,								// LZW data compression
		ct_LZH,

} ct_TYPES;

//
//  	FILE CHUNK HEADER FORMATS
//

struct COMPStruct
{
	Uint32 DecompLen;
};


struct JAMPHeader
{
	Uint32 OrginalLen;			// Orginal FileLength of compressed Data.
	ct_TYPES CompType;					// SEE: ct_TYPES above for list of pos.
	Uint32 CompressLen;			// Length of data after compression (A MUST for LZHUFF!)
};
