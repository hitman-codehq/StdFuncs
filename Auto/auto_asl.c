
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *AslBase;

void __open_AslBase() __attribute__((constructor));
void __close_AslBase() __attribute__((destructor));

void __open_AslBase()
{
	AslBase = SafeOpenLibrary("asl.library");
}

void __close_AslBase()
{
	CloseLibrary(AslBase);
}
