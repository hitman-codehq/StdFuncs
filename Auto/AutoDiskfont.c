
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *DiskfontBase;

void OpenDiskfontBase() __attribute__((constructor));
void CloseDiskfontBase() __attribute__((destructor));

void OpenDiskfontBase()
{
	DiskfontBase = SafeOpenLibrary("diskfont.library");
}

void CloseDiskfontBase()
{
	CloseLibrary(DiskfontBase);
}
