
#include <proto/exec.h>

#define MINIMUM_VERSION 40

//#pragma weak AslBase

__attribute__((weak)) struct Library *AslBase;
//struct Library *AslBase;

void __open_AslBase() __attribute__((constructor));
void __close_AslBase() __attribute__((destructor));

void __open_AslBase()
{
	AslBase = OpenLibrary("asl.library", MINIMUM_VERSION);
}

void __close_AslBase()
{
	CloseLibrary(AslBase);
}
