
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *CheckBoxBase;

void OpenCheckBoxBase() __attribute__((constructor));
void CloseCheckBoxBase() __attribute__((destructor));

void OpenCheckBoxBase()
{
	CheckBoxBase = SafeOpenLibrary("Gadgets/checkbox.gadget");
}

void CloseCheckBoxBase()
{
	CloseLibrary(CheckBoxBase);
}
