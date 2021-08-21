
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *IconBase;

void OpenIconBase() __attribute__((constructor));
void CloseIconBase() __attribute__((destructor));

void OpenIconBase()
{
	IconBase = SafeOpenLibrary("icon.library");
}

void CloseIconBase()
{
	CloseLibrary(IconBase);
}
