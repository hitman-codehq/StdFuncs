
#include <proto/exec.h>

#define MINIMUM_VERSION 40

struct Library *BitMapBase;

void __open_BitMapBase() __attribute__((constructor));
void __close_BitMapBase() __attribute__((destructor));

void __open_BitMapBase()
{
	BitMapBase = OpenLibrary("Images/bitmap.image", MINIMUM_VERSION);
}

void __close_BitMapBase()
{
	CloseLibrary(BitMapBase);
}
