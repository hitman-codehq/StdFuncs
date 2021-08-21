
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *ScrollerBase;

void __open_ScrollerBase() __attribute__((constructor));
void __close_ScrollerBase() __attribute__((destructor));

void __open_ScrollerBase()
{
	ScrollerBase = SafeOpenLibrary("Gadgets/scroller.gadget");
}

void __close_ScrollerBase()
{
	CloseLibrary(ScrollerBase);
}
