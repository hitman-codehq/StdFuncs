
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *LabelBase;

void __open_LabelBase() __attribute__((constructor));
void __close_LabelBase() __attribute__((destructor));

void __open_LabelBase()
{
	LabelBase = SafeOpenLibrary("Images/label.image");
}

void __close_LabelBase()
{
	CloseLibrary(LabelBase);
}
