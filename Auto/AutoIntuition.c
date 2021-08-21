
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *IntuitionBase;

void __open_IntuitionBase() __attribute__((constructor));
void __close_IntuitionBase() __attribute__((destructor));

void __open_IntuitionBase()
{
	IntuitionBase = SafeOpenLibrary("intuition.library");
}

void __close_IntuitionBase()
{
	CloseLibrary(IntuitionBase);
}
