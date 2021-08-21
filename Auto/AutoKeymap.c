
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *KeymapBase;

void OpenKeymapBase() __attribute__((constructor));
void CloseKeymapBase() __attribute__((destructor));

void OpenKeymapBase()
{
	KeymapBase = SafeOpenLibrary("keymap.library");
}

void CloseKeymapBase()
{
	CloseLibrary(KeymapBase);
}
