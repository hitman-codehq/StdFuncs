
#include <proto/exec.h>

#define MINIMUM_VERSION 40

struct Library *LayoutBase;

void __open_LayoutBase() __attribute__((constructor));
void __close_LayoutBase() __attribute__((destructor));

void __open_LayoutBase()
{
	LayoutBase = OpenLibrary("Gadgets/layout.gadget", MINIMUM_VERSION);
}

void __close_LayoutBase()
{
	CloseLibrary(LayoutBase);
}
