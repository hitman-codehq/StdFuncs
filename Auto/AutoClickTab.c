
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *ClickTabBase;

void OpenClickTabBase() __attribute__((constructor));
void CloseClickTabBase() __attribute__((destructor));

void OpenClickTabBase()
{
	ClickTabBase = SafeOpenLibrary("Gadgets/clicktab.gadget");
}

void CloseClickTabBase()
{
	CloseLibrary(ClickTabBase);
}
