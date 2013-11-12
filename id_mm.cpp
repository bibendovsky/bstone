#include "id_mm.h"

#include <cstdlib>


void* bufferseg = NULL;


void MM_Startup()
{
    MM_Shutdown();

    bufferseg = new char[BUFFERSIZE];
}

void MM_Shutdown()
{
    delete [] static_cast<char*>(bufferseg);
    bufferseg = NULL;
}
