
#include <proto/exec.h>

#define MINIMUM_VERSION 40

struct Library *GadToolsBase;

void __open_GadToolsBase() __attribute__((constructor));
void __close_GadToolsBase() __attribute__((destructor));

void __open_GadToolsBase()
{
	GadToolsBase = OpenLibrary("gadtools.library", MINIMUM_VERSION);
}

void __close_GadToolsBase()
{
	CloseLibrary(GadToolsBase);
}
