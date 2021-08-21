
#include <proto/exec.h>

#define MINIMUM_VERSION 40

struct Library *DataTypesBase;

void __open_DataTypesBase() __attribute__((constructor));
void __close_DataTypesBase() __attribute__((destructor));

void __open_DataTypesBase()
{
	DataTypesBase = OpenLibrary("datatypes.library", MINIMUM_VERSION);
}

void __close_DataTypesBase()
{
	CloseLibrary(DataTypesBase);
}
