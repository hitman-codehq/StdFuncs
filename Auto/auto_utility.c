
#include <proto/exec.h>

#define MINIMUM_VERSION 40

__attribute__((weak)) struct Library *UtilityBase;

void __open_UtilityBase() __attribute__((constructor));
void __close_UtilityBase() __attribute__((destructor));

void __open_UtilityBase()
{
	UtilityBase = OpenLibrary("utility.library", MINIMUM_VERSION);
}

void __close_UtilityBase()
{
	CloseLibrary(UtilityBase);
}
