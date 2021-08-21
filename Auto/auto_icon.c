
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *IconBase;

void __open_IconBase() __attribute__((constructor));
void __close_IconBase() __attribute__((destructor));

void __open_IconBase()
{
	IconBase = SafeOpenLibrary("icon.library");
}

void __close_IconBase()
{
	CloseLibrary(IconBase);
}
