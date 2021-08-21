
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *GadToolsBase;

void OpenGadToolsBase() __attribute__((constructor));
void CloseGadToolsBase() __attribute__((destructor));

void OpenGadToolsBase()
{
	GadToolsBase = SafeOpenLibrary("gadtools.library");
}

void CloseGadToolsBase()
{
	CloseLibrary(GadToolsBase);
}
