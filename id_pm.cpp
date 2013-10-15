#include "id_pm.h"

#include "id_heads.h"


// File specific variables
char PageFileName[13] = {"VSWAP."};

int ChunksInFile = 0;
int PMSpriteStart = 0;
int PMSoundStart = 0;


namespace {


bstone::FileStream PageFile;
Uint8* raw_data = NULL;
Uint32* chunks_offsets = NULL;


} // namespace


static void open_page_file(
    const char* file_name)
{
    PageFile.open(file_name);

    if (!PageFile.is_open())
        PM_ERROR(PML_OPENPAGEFILE_OPEN);

    Sint64 file_length = PageFile.get_size();

    if (file_length > 4 * 1024 * 1024)
        ::Quit("Page file is too large.");

    Sint32 file_length_32 = static_cast<Sint32>(file_length);

    raw_data = new Uint8[file_length_32 + PMPageSize];
    std::uninitialized_fill_n(&raw_data[file_length], PMPageSize, 0);

    if (PageFile.read(raw_data, file_length_32) != file_length_32)
        PM_ERROR(PML_READFROMFILE_READ);

    bstone::MemoryBinaryReader reader(raw_data, file_length);

    ChunksInFile = bstone::Endian::le(reader.read_u16());
    PMSpriteStart = bstone::Endian::le(reader.read_u16());
    PMSoundStart = bstone::Endian::le(reader.read_u16());

    chunks_offsets = reinterpret_cast<Uint32*>(&raw_data[6]);
    bstone::Endian::lei(chunks_offsets, ChunksInFile + 1);
}

void PM_Startup()
{
    ::PM_Shutdown();
    ::open_page_file(PageFileName);
}

void PM_Shutdown()
{
    PageFile.close();

    ChunksInFile = 0;
    PMSpriteStart = 0;
    PMSoundStart = 0;

    delete [] raw_data;
    raw_data = NULL;

    chunks_offsets = NULL;
}

void* PM_GetPage(
    int page_number)
{
    if (page_number >= ChunksInFile)
        PM_ERROR(PM_GETPAGE_BAD_PAGE);

    Uint32 offset = chunks_offsets[page_number];

    if (offset == 0)
        PM_ERROR(PM_GETPAGE_SPARSE_PAGE);

    return &raw_data[offset];
}

void* PM_GetSoundPage(
    int page_number)
{
    return PM_GetPage(PMSoundStart + page_number);
}

void* PM_GetSpritePage(
    int page_number)
{
    return PM_GetPage(PMSpriteStart + page_number);
}
