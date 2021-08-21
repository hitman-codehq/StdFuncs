
#include <proto/exec.h>

#define MINIMUM_VERSION 40

struct Library *LabelBase;

void __open_LabelBase() __attribute__((constructor));
void __close_LabelBase() __attribute__((destructor));

void __open_LabelBase()
{
	LabelBase = OpenLibrary("Images/label.image", MINIMUM_VERSION);
}

void __close_LabelBase()
{
	CloseLibrary(LabelBase);
}
