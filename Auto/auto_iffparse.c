
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *IFFParseBase;

void __open_IFFParseBase() __attribute__((constructor));
void __close_IFFParseBase() __attribute__((destructor));

void __open_IFFParseBase()
{
	IFFParseBase = SafeOpenLibrary("iffparse.library");
}

void __close_IFFParseBase()
{
	CloseLibrary(IFFParseBase);
}
