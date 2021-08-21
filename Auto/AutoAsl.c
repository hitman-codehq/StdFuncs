
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *AslBase;

void OpenAslBase() __attribute__((constructor));
void CloseAslBase() __attribute__((destructor));

void OpenAslBase()
{
	AslBase = SafeOpenLibrary("asl.library");
}

void CloseAslBase()
{
	CloseLibrary(AslBase);
}
