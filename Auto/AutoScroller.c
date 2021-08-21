
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *ScrollerBase;

void OpenScrollerBase() __attribute__((constructor));
void CloseScrollerBase() __attribute__((destructor));

void OpenScrollerBase()
{
	ScrollerBase = SafeOpenLibrary("Gadgets/scroller.gadget");
}

void CloseScrollerBase()
{
	CloseLibrary(ScrollerBase);
}
