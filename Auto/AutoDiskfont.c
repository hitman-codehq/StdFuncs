
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *DiskfontBase;

void __open_DiskfontBase() __attribute__((constructor));
void __close_DiskfontBase() __attribute__((destructor));

void __open_DiskfontBase()
{
	DiskfontBase = SafeOpenLibrary("diskfont.library");
}

void __close_DiskfontBase()
{
	CloseLibrary(DiskfontBase);
}
