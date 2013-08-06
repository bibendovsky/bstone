#include "id_pm.h"

#include "id_heads.h"


// File specific variables
char PageFileName[13] = {"VSWAP."};

static int PageFile = -1;
unsigned short ChunksInFile;
unsigned short PMSpriteStart;
unsigned short PMSoundStart;

static unsigned char* raw_data;
static unsigned long* chunks_offsets;


static void open_page_file(const char* filename)
{
    long file_length;

    PageFile = open(filename, O_RDONLY + O_BINARY);

    if (PageFile == -1)
        PM_ERROR(PML_OPENPAGEFILE_OPEN);

    file_length = filelength(PageFile);

    raw_data = (unsigned char*)malloc(file_length + PMPageSize);
    memset(&raw_data[file_length], 0, PMPageSize);

    if (read(PageFile, raw_data, file_length) != file_length)
        PM_ERROR(PML_READFROMFILE_READ);

    ChunksInFile = ((unsigned short*)raw_data)[0];
    PMSpriteStart = ((unsigned short*)raw_data)[1];
    PMSoundStart = ((unsigned short*)raw_data)[2];

    chunks_offsets = (unsigned long*)&raw_data[6];
}

void PM_Startup()
{
    PM_Shutdown();

    open_page_file(PageFileName);
}

void PM_Shutdown()
{
    if (PageFile >= 0) {
        close(PageFile);
        PageFile = -1;
    }

    ChunksInFile = 0;
    PMSpriteStart = 0;
    PMSoundStart = 0;

    free(raw_data);
    raw_data = NULL;

    chunks_offsets = NULL;
}

void* PM_GetPage(int page_number)
{
    unsigned long offset;

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
