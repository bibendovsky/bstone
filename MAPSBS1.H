///////////////////////////////////////
//
// TED5 Map Header for BS1
//
///////////////////////////////////////

//
// Map Names
//
typedef enum {
		GAME1_BASEMENT_MAP,      // 0
		GAME1_FLOOR1_MAP,        // 1
		GAME1_FLOOR2_MAP,        // 2
		GAME1_FLOOR3_MAP,        // 3
		GAME1_FLOOR4_MAP,        // 4
		GAME1_FLOOR5_MAP,        // 5
		GAME1_FLOOR6_MAP,        // 6
		GAME1_FLOOR7_MAP,        // 7
		GAME1_FLOOR8_MAP,        // 8
		GAME1_FLOOR9_MAP,        // 9
		GAME1_FLOOR10_MAP,       // 10
		GAME1_SPACE_MAP,         // 11
		GAME1_SECRET2_MAP,       // 12
		GAME1_SECRET3_MAP,       // 13
		GAME1_SECRET4_MAP,       // 14
		GAME2_BASEMENT_MAP,      // 15
		GAME2_FLOOR1_MAP,        // 16
		GAME2_FLOOR2_MAP,        // 17
		GAME2_FLOOR3_MAP,        // 18
		GAME2_FLOOR4_MAP,        // 19
		GAME2_FLOOR5_MAP,        // 20
		GAME2_FLOOR6_MAP,        // 21
		GAME2_FLOOR7_MAP,        // 22
		GAME2_FLOOR8_MAP,        // 23
		GAME2_FLOOR9_MAP,        // 24
		GAME2_FLOOR10_MAP,       // 25
		GAME2_SECRET1_MAP,       // 26
		GAME2_SECRET2_MAP,       // 27
		GAME2_SECRET3_MAP,       // 28
		GAME2_SECRET4_MAP,       // 29
		GAME3_BASEMENT_MAP,      // 30
		GAME3_FLOOR1_MAP,        // 31
		GAME3_FLOOR2_MAP,        // 32
		GAME3_FLOOR3_MAP,        // 33
		GAME3_FLOOR4_MAP,        // 34
		GAME3_FLOOR5_MAP,        // 35
		GAME3_FLOOR6_MAP,        // 36
		GAME3_FLOOR7_MAP,        // 37
		GAME3_FLOOR8_MAP,        // 38
		GAME3_FLOOR9_MAP,        // 39
		GAME3_FLOOR10_MAP,       // 40
		GAME3_SECRET1_MAP,       // 41
		GAME3_SECRET2_MAP,       // 42
		GAME3_SECRET3_MAP,       // 43
		GAME3_SECRET4_MAP,       // 44
		GAME4_BASEMENT_MAP,      // 45
		GAME4_FLOOR1_MAP,        // 46
		GAME4_FLOOR2_MAP,        // 47
		GAME4_FLOOR3_MAP,        // 48
		GAME4_FLOOR4_MAP,        // 49
		GAME4_FLOOR5_MAP,        // 50
		GAME4_FLOOR6_MAP,        // 51
		GAME4_FLOOR7_MAP,        // 52
		GAME4_FLOOR8_MAP,        // 53
		GAME4_FLOOR9_MAP,        // 54
		GAME4_FLOOR10_MAP,       // 55
		GAME4_SECRET1_MAP,       // 56
		GAME4_SECRET2_MAP,       // 57
		GAME4_SECRET3_MAP,       // 58
		GAME4_SECRET4_MAP,       // 59
		GAME5_BASEMENT_MAP,      // 60
		GAME5_FLOOR1_MAP,        // 61
		GAME5_FLOOR2_MAP,        // 62
		GAME5_FLOOR3_MAP,        // 63
		GAME5_FLOOR4_MAP,        // 64
		GAME5_FLOOR5_MAP,        // 65
		GAME5_FLOOR6_MAP,        // 66
		GAME5_FLOOR7_MAP,        // 67
		GAME5_FLOOR8_MAP,        // 68
		GAME5_FLOOR9_MAP,        // 69
		GAME5_FLOOR10_MAP,       // 70
		GAME5_SECRET1_MAP,       // 71
		GAME5_SECRET2_MAP,       // 72
		GAME5_SECRET3_MAP,       // 73
		GAME5_SECRET4_MAP,       // 74
		GAME6_BASEMENT_MAP,      // 75
		GAME6_FLOOR1_MAP,        // 76
		GAME6_FLOOR2_MAP,        // 77
		GAME6_FLOOR3_MAP,        // 78
		GAME6_FLOOR4_MAP,        // 79
		GAME6_FLOOR5_MAP,        // 80
		GAME6_FLOOR6_MAP,        // 81
		GAME6_FLOOR7_MAP,        // 82
		GAME6_FLOOR8_MAP,        // 83
		GAME6_FLOOR9_MAP,        // 84
		GAME6_FLOOR10_MAP,       // 85
		GAME6_SECRET1_MAP,       // 86
		GAME6_SECRET2_MAP,       // 87
		GAME6_SECRET3_MAP,       // 88
		GAME6_SECRET4_MAP,       // 89
		LASTMAP
	     } mapnames;

//
// TILEINFO offsets
//
#define ANIM		402
#define SPEED		(ANIM+NUMTILE16)
