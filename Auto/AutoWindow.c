
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *WindowBase;

void OpenWindowBase() __attribute__((constructor));
void CloseWindowBase() __attribute__((destructor));

void OpenWindowBase()
{
	WindowBase = SafeOpenLibrary("window.class");
}

void CloseWindowBase()
{
	CloseLibrary(WindowBase);
}
