#ifndef ID_PM_H
#define ID_PM_H


const int PMPageSize = 4096;


extern int ChunksInFile;
extern int PMSpriteStart;
extern int PMSoundStart;

extern char PageFileName[13];


void PM_Startup();
void PM_Shutdown();
void* PM_GetPage(int page_number);
void* PM_GetSoundPage(int page_number);
void* PM_GetSpritePage(int page_number);


#endif // ID_PM_H
