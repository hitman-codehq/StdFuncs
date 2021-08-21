
#include <proto/exec.h>

#define MINIMUM_VERSION 40

struct Library *ScrollerBase;

void __open_ScrollerBase() __attribute__((constructor));
void __close_ScrollerBase() __attribute__((destructor));

void __open_ScrollerBase()
{
	ScrollerBase = OpenLibrary("Gadgets/scroller.gadget", MINIMUM_VERSION);
}

void __close_ScrollerBase()
{
	CloseLibrary(ScrollerBase);
}
