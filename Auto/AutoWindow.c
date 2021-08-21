
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *WindowBase;

// TODO: CAW - Rename these
void __open_WindowBase() __attribute__((constructor));
void __close_WindowBase() __attribute__((destructor));

void __open_WindowBase()
{
	WindowBase = SafeOpenLibrary("window.class");
}

void __close_WindowBase()
{
	CloseLibrary(WindowBase);
}
