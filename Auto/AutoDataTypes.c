
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *DataTypesBase;

void OpenDataTypesBase() __attribute__((constructor));
void CloseDataTypesBase() __attribute__((destructor));

void OpenDataTypesBase()
{
	DataTypesBase = SafeOpenLibrary("datatypes.library");
}

void CloseDataTypesBase()
{
	CloseLibrary(DataTypesBase);
}
