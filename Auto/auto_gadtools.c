
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *GadToolsBase;

void __open_GadToolsBase() __attribute__((constructor));
void __close_GadToolsBase() __attribute__((destructor));

void __open_GadToolsBase()
{
	GadToolsBase = SafeOpenLibrary("gadtools.library");
}

void __close_GadToolsBase()
{
	CloseLibrary(GadToolsBase);
}
