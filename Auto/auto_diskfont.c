
#include <proto/exec.h>

#define MINIMUM_VERSION 40

__attribute__((weak)) struct Library *DiskfontBase;

void __open_DiskfontBase() __attribute__((constructor));
void __close_DiskfontBase() __attribute__((destructor));

void __open_DiskfontBase()
{
	DiskfontBase = OpenLibrary("diskfont.library", MINIMUM_VERSION);
}

void __close_DiskfontBase()
{
	CloseLibrary(DiskfontBase);
}
