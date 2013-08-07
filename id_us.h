//
//	ID Engine
//	ID_US.h - Header file for the User Manager
//	v1.0d1
//	By Jason Blochowiak
//

#ifndef	__ID_US__
#define	__ID_US__

#ifdef	__DEBUG__
#define	__DEBUG_UserMgr__
#endif

//#define	HELPTEXTLINKED

#define	MaxX	320
#define	MaxY	200

#define	MaxHelpLines	500

#define	MaxHighName	57
#define	MaxScores	10
typedef	struct
		{
			char	name[MaxHighName + 1];
			long	score;
			word	completed,episode,ratio;
		} HighScore;

#define	MaxGameName		32
#define	MaxSaveGames	6
typedef	struct
		{
			char	signature[4];
			word	*oldtest;
			boolean	present;
			char	name[MaxGameName + 1];
		} SaveGame;

#define	MaxString	128	// Maximum input string size

typedef	struct
		{
			short	x,y,
				w,h,
				px,py;
		} WindowRec;	// Record used to save & restore screen windows

typedef	enum
		{
			gd_Continue,
			gd_Easy,
			gd_Normal,
			gd_Hard
		} GameDiff;

// Custom Cursor struct type for US_LineInput()

typedef struct								  // JAM - Custom Cursor Support
{
	char cursor_char;
	char do_not_use;						 // Space holder for ASCZ string
	unsigned short cursor_color;
	unsigned short font_number;
} US_CursorStruct;

//	Hack import for TED launch support
//extern	boolean		tedlevel;
//extern	word		tedlevelnum;		 
extern	void		TEDDeath(void);

extern	boolean		ingame,		// Set by game code if a game is in progress
					abortgame,	// Set if a game load failed
					loadedgame,	// Set if the current game was loaded
					NoWait,
					HighScoresDirty;
extern	char		*abortprogram;	// Set to error msg if program is dying
extern	GameDiff	restartgame;	// Normally gd_Continue, else starts game
extern	word		PrintX,PrintY;	// Current printing location in the window
extern	word		WindowX,WindowY,// Current location of window
					WindowW,WindowH;// Current size of window

extern	boolean		Button0,Button1,
					CursorBad;
extern	short			CursorX,CursorY;

extern	void		(*USL_MeasureString)(char *,word *,word *),
					(*USL_DrawString)(char *);

extern	boolean		(*USL_SaveGame)(short),(*USL_LoadGame)(short);
extern	void		(*USL_ResetGame)(void);
extern	SaveGame	Games[MaxSaveGames];
extern	HighScore	Scores[];

extern US_CursorStruct US_CustomCursor;		// JAM
extern boolean use_custom_cursor;				// JAM

#define	US_HomeWindow()	{PrintX = WindowX; PrintY = WindowY;}

extern	void	US_Startup(void),
				US_Setup(void),
				US_Shutdown(void),
				US_InitRndT(boolean randomize),
				US_SetLoadSaveHooks(boolean (*load)(short),
									boolean (*save)(short),
									void (*reset)(void)),
				US_TextScreen(void),
				US_UpdateTextScreen(void),
				US_FinishTextScreen(void),
				US_DrawWindow(word x,word y,word w,word h),
				US_CenterWindow(word,word),
				US_SaveWindow(WindowRec *win),
				US_RestoreWindow(WindowRec *win),
				US_ClearWindow(void),
				US_SetPrintRoutines(void (*measure)(char *,word *,word *),
									void (*print)(char *)),
				US_PrintCentered(char *s),
				US_CPrint(char *s),
				US_CPrintLine(char *s),
				US_Print(char *s),
				US_PrintUnsigned(longword n),
				US_PrintSigned(long n),
				US_StartCursor(void),
				US_ShutCursor(void),
				US_CheckHighScore(long score,word other),
				US_DisplayHighScores(short which);
extern	boolean	US_UpdateCursor(void),
				US_LineInput(short x,short y,char *buf,char *def,boolean escok,
								short maxchars,short maxwidth);
extern	short		US_CheckParm(char *parm,char **strings),

				US_RndT(void);

		void	USL_PrintInCenter(char *s,Rect r);
		char 	*USL_GiveSaveName(word game);
#endif
