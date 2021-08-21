
#include <proto/exec.h>

#define MINIMUM_VERSION 40

struct Library *StringBase;

void __open_StringBase() __attribute__((constructor));
void __close_StringBase() __attribute__((destructor));

void __open_StringBase()
{
	StringBase = OpenLibrary("Gadgets/string.gadget", MINIMUM_VERSION);
}

void __close_StringBase()
{
	CloseLibrary(StringBase);
}
