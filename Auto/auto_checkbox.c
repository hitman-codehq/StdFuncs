
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *CheckBoxBase;

void __open_CheckBoxBase() __attribute__((constructor));
void __close_CheckBoxBase() __attribute__((destructor));

void __open_CheckBoxBase()
{
	CheckBoxBase = SafeOpenLibrary("Gadgets/checkbox.gadget");
}

void __close_CheckBoxBase()
{
	CloseLibrary(CheckBoxBase);
}
