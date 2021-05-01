
#include <proto/exec.h>

#define MINIMUM_VERSION 40

__attribute__((weak)) struct Library *IconBase;

void __open_IconBase() __attribute__((constructor));
void __close_IconBase() __attribute__((destructor));

void __open_IconBase()
{
	IconBase = OpenLibrary("icon.library", MINIMUM_VERSION);
}

void __close_IconBase()
{
	CloseLibrary(IconBase);
}
