
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *UtilityBase;

void OpenUtilityBase() __attribute__((constructor));
void CloseUtilityBase() __attribute__((destructor));

void OpenUtilityBase()
{
	UtilityBase = SafeOpenLibrary("utility.library");
}

void CloseUtilityBase()
{
	CloseLibrary(UtilityBase);
}
