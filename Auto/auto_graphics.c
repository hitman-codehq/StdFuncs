
#include <proto/exec.h>

#define MINIMUM_VERSION 40

struct Library *GfxBase;

void __open_GfxBase() __attribute__((constructor));
void __close_GfxBase() __attribute__((destructor));

void __open_GfxBase()
{
	GfxBase = OpenLibrary("graphics.library", MINIMUM_VERSION);
}

void __close_GfxBase()
{
	CloseLibrary(GfxBase);
}
