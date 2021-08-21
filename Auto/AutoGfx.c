
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *GfxBase;

void OpenGfxBase() __attribute__((constructor));
void CloseGfxBase() __attribute__((destructor));

void OpenGfxBase()
{
	GfxBase = SafeOpenLibrary("graphics.library");
}

void CloseGfxBase()
{
	CloseLibrary(GfxBase);
}
