
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *UtilityBase;

void __open_UtilityBase() __attribute__((constructor));
void __close_UtilityBase() __attribute__((destructor));

void __open_UtilityBase()
{
	UtilityBase = SafeOpenLibrary("utility.library");
}

void __close_UtilityBase()
{
	CloseLibrary(UtilityBase);
}
