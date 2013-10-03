#include "id_pm.h"

#include "id_heads.h"


// File specific variables
char PageFileName[13] = {"VSWAP."};

static bstone::FileStream PageFile;
Uint16 ChunksInFile;
Uint16 PMSpriteStart;
Uint16 PMSoundStart;

static Uint8* raw_data;
static Uint32* chunks_offsets;


static void open_page_file(const char* filename)
{
    Sint32 file_length;

    PageFile.open(filename);

    if (!PageFile.is_open())
        PM_ERROR(PML_OPENPAGEFILE_OPEN);

    file_length = static_cast<Sint32>(PageFile.get_size());

    raw_data = (Uint8*)malloc(file_length + PMPageSize);
    memset(&raw_data[file_length], 0, PMPageSize);

    if (PageFile.read(raw_data, file_length) != file_length)
        PM_ERROR(PML_READFROMFILE_READ);

    ChunksInFile = ((Uint16*)raw_data)[0];
    PMSpriteStart = ((Uint16*)raw_data)[1];
    PMSoundStart = ((Uint16*)raw_data)[2];

    chunks_offsets = (Uint32*)&raw_data[6];
}

void PM_Startup()
{
    PM_Shutdown();

    open_page_file(PageFileName);
}

void PM_Shutdown()
{
    PageFile.close();

    ChunksInFile = 0;
    PMSpriteStart = 0;
    PMSoundStart = 0;

    free(raw_data);
    raw_data = NULL;

    chunks_offsets = NULL;
}

void* PM_GetPage(int page_number)
{
    Uint32 offset;

    if (page_number >= ChunksInFile)
        PM_ERROR(PM_GETPAGE_BAD_PAGE);

    offset = chunks_offsets[page_number];

    if (offset == 0)
        PM_ERROR(PM_GETPAGE_SPARSE_PAGE);

    return &raw_data[offset];
}

void* PM_GetSoundPage(int page_number)
{
    return PM_GetPage(PMSoundStart + page_number);
}

void* PM_GetSpritePage(int page_number)
{
    return PM_GetPage(PMSpriteStart + page_number);
}
