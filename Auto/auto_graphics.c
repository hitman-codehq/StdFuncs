
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *GfxBase;

void __open_GfxBase() __attribute__((constructor));
void __close_GfxBase() __attribute__((destructor));

void __open_GfxBase()
{
	GfxBase = SafeOpenLibrary("graphics.library");
}

void __close_GfxBase()
{
	CloseLibrary(GfxBase);
}
