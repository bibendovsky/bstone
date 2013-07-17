///////////////////////////////////////
//
// TED5 Map Header for FIR
//
///////////////////////////////////////

//
// Map Names
//
typedef enum {
		AREA0_MAP,               // 0
		AREA1_MAP,               // 1
		AREA2_MAP,               // 2
		AREA3_MAP,               // 3
		AREA4_MAP,               // 4
		AREA5_MAP,               // 5
		AREA6_MAP,               // 6
		AREA7_MAP,               // 7
		AREA8_MAP,               // 8
		AREA9_MAP,               // 9
		AREA10_MAP,              // 10
		AREA11_MAP,              // 11
		AREA12_MAP,              // 12
		AREA13_MAP,              // 13
		AREA14_MAP,              // 14
		AREA15_MAP,              // 15
		AREA16_MAP,              // 16
		AREA17_MAP,              // 17
		AREA18_MAP,              // 18
		AREA19_MAP,              // 19
		SECRET1_MAP,             // 20
		SECRET2_MAP,             // 21
		SECRET3_MAP,             // 22
		SECRET4_MAP,             // 23
		LASTMAP
	     } mapnames;

//
// TILEINFO offsets
//
#define ANIM		402
#define SPEED		(ANIM+NUMTILE16)