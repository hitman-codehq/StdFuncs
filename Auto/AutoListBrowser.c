
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *ListBrowserBase;

void OpenListBrowserBase() __attribute__((constructor));
void CloseListBrowserBase() __attribute__((destructor));

void OpenListBrowserBase()
{
	ListBrowserBase = SafeOpenLibrary("Gadgets/listbrowser.gadget");
}

void CloseListBrowserBase()
{
	CloseLibrary(ListBrowserBase);
}
