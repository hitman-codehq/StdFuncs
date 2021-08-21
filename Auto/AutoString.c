
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *StringBase;

void OpenStringBase() __attribute__((constructor));
void CloseStringBase() __attribute__((destructor));

void OpenStringBase()
{
	StringBase = SafeOpenLibrary("Gadgets/string.gadget");
}

void CloseStringBase()
{
	CloseLibrary(StringBase);
}
