
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *StringBase;

void __open_StringBase() __attribute__((constructor));
void __close_StringBase() __attribute__((destructor));

void __open_StringBase()
{
	StringBase = SafeOpenLibrary("Gadgets/string.gadget");
}

void __close_StringBase()
{
	CloseLibrary(StringBase);
}
