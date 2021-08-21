
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *LayoutBase;

void OpenLayoutBase() __attribute__((constructor));
void CloseLayoutBase() __attribute__((destructor));

void OpenLayoutBase()
{
	LayoutBase = SafeOpenLibrary("Gadgets/layout.gadget");
}

void CloseLayoutBase()
{
	CloseLibrary(LayoutBase);
}
