
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *LabelBase;

void OpenLabelBase() __attribute__((constructor));
void CloseLabelBase() __attribute__((destructor));

void OpenLabelBase()
{
	LabelBase = SafeOpenLibrary("Images/label.image");
}

void CloseLabelBase()
{
	CloseLibrary(LabelBase);
}
