
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *BitMapBase;

void __open_BitMapBase() __attribute__((constructor));
void __close_BitMapBase() __attribute__((destructor));

void __open_BitMapBase()
{
	BitMapBase = SafeOpenLibrary("Images/bitmap.image");
}

void __close_BitMapBase()
{
	CloseLibrary(BitMapBase);
}
