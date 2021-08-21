
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *IntuitionBase;

void OpenIntuitionBase() __attribute__((constructor));
void CloseIntuitionBase() __attribute__((destructor));

void OpenIntuitionBase()
{
	IntuitionBase = SafeOpenLibrary("intuition.library");
}

void CloseIntuitionBase()
{
	CloseLibrary(IntuitionBase);
}
