
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *KeymapBase;

void __open_KeymapBase() __attribute__((constructor));
void __close_KeymapBase() __attribute__((destructor));

void __open_KeymapBase()
{
	KeymapBase = SafeOpenLibrary("keymap.library");
}

void __close_KeymapBase()
{
	CloseLibrary(KeymapBase);
}
