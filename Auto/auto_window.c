
#include <proto/exec.h>

// TODO: CAW - Define this somewhere else or in a variable
#define MINIMUM_VERSION 40

struct Library *WindowBase;

// TODO: CAW - Rename these
void __open_WindowBase() __attribute__((constructor));
void __close_WindowBase() __attribute__((destructor));

void __open_WindowBase()
{
	WindowBase = OpenLibrary("window.class", MINIMUM_VERSION);
	// TODO: CAW - Automatic message if this fails
}

void __close_WindowBase()
{
	CloseLibrary(WindowBase);
}
