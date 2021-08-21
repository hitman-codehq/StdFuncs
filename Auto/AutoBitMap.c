
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *BitMapBase;

void OpenBitMapBase() __attribute__((constructor));
void CloseBitMapBase() __attribute__((destructor));

void OpenBitMapBase()
{
	BitMapBase = SafeOpenLibrary("Images/bitmap.image");
}

void CloseBitMapBase()
{
	CloseLibrary(BitMapBase);
}
