
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *ListBrowserBase;

void __open_ListBrowserBase() __attribute__((constructor));
void __close_ListBrowserBase() __attribute__((destructor));

void __open_ListBrowserBase()
{
	ListBrowserBase = SafeOpenLibrary("Gadgets/listbrowser.gadget");
}

void __close_ListBrowserBase()
{
	CloseLibrary(ListBrowserBase);
}
