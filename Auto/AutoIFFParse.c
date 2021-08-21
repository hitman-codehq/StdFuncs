
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *IFFParseBase;

void OpenIFFParseBase() __attribute__((constructor));
void CloseIFFParseBase() __attribute__((destructor));

void OpenIFFParseBase()
{
	IFFParseBase = SafeOpenLibrary("iffparse.library");
}

void CloseIFFParseBase()
{
	CloseLibrary(IFFParseBase);
}
