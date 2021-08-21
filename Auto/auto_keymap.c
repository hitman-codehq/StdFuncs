
#include <proto/exec.h>

#define MINIMUM_VERSION 40

struct Library *KeymapBase;

void __open_KeymapBase() __attribute__((constructor));
void __close_KeymapBase() __attribute__((destructor));

void __open_KeymapBase()
{
	KeymapBase = OpenLibrary("keymap.library", MINIMUM_VERSION);
}

void __close_KeymapBase()
{
	CloseLibrary(KeymapBase);
}
