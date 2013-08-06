#ifndef ID_PM_H
#define ID_PM_H


#define PMPageSize (4096)


extern unsigned short ChunksInFile;
extern unsigned short PMSpriteStart;
extern unsigned short PMSoundStart;

extern char PageFileName[13];

void PM_Startup();
void PM_Shutdown();
void* PM_GetPage(int page_number);
void* PM_GetSoundPage(int page_number);
void* PM_GetSpritePage(int page_number);


#endif // ID_PM_H
