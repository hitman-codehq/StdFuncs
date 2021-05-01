
#include <proto/exec.h>

#define MINIMUM_VERSION 40

__attribute__((weak)) struct Library *CheckBoxBase;

void __open_CheckBoxBase() __attribute__((constructor));
void __close_CheckBoxBase() __attribute__((destructor));

void __open_CheckBoxBase()
{
	CheckBoxBase = OpenLibrary("Gadgets/checkbox.gadget", MINIMUM_VERSION);
}

void __close_CheckBoxBase()
{
	CloseLibrary(CheckBoxBase);
}
