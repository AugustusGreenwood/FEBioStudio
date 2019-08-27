#include "stdafx.h"
#include "memtool.h"

#ifdef WIN32
#include <memory>
#else
#include <memory.h>
#endif

using namespace std;

void mread(void* pdest, size_t Size, size_t Cnt, void** psrc)
{
	size_t nsize = Size*Cnt;
	memcpy(pdest, *psrc, nsize);

	char*& pc = (char*&)(*psrc);
	pc += nsize;
}
