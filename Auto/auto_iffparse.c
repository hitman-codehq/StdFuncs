
#include <proto/exec.h>

#define MINIMUM_VERSION 40

__attribute__((weak)) struct Library *IFFParseBase;

void __open_IFFParseBase() __attribute__((constructor));
void __close_IFFParseBase() __attribute__((destructor));

void __open_IFFParseBase()
{
	IFFParseBase = OpenLibrary("iffparse.library", MINIMUM_VERSION);
}

void __close_IFFParseBase()
{
	CloseLibrary(IFFParseBase);
}
