
#include <proto/exec.h>

#define MINIMUM_VERSION 40

__attribute__((weak)) struct Library *IntuitionBase;

void __open_IntuitionBase() __attribute__((constructor));
void __close_IntuitionBase() __attribute__((destructor));

void __open_IntuitionBase()
{
	IntuitionBase = OpenLibrary("intuition.library", MINIMUM_VERSION);
}

void __close_IntuitionBase()
{
	CloseLibrary(IntuitionBase);
}
