
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *LayoutBase;

void __open_LayoutBase() __attribute__((constructor));
void __close_LayoutBase() __attribute__((destructor));

void __open_LayoutBase()
{
	LayoutBase = SafeOpenLibrary("Gadgets/layout.gadget");
}

void __close_LayoutBase()
{
	CloseLibrary(LayoutBase);
}
