
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *DataTypesBase;

void __open_DataTypesBase() __attribute__((constructor));
void __close_DataTypesBase() __attribute__((destructor));

void __open_DataTypesBase()
{
	DataTypesBase = SafeOpenLibrary("datatypes.library");
}

void __close_DataTypesBase()
{
	CloseLibrary(DataTypesBase);
}
