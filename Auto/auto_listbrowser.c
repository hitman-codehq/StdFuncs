
#include <proto/exec.h>

#define MINIMUM_VERSION 40

struct Library *ListBrowserBase;

void __open_ListBrowserBase() __attribute__((constructor));
void __close_ListBrowserBase() __attribute__((destructor));

void __open_ListBrowserBase()
{
	ListBrowserBase = OpenLibrary("Gadgets/listbrowser.gadget", MINIMUM_VERSION);
}

void __close_ListBrowserBase()
{
	CloseLibrary(ListBrowserBase);
}
