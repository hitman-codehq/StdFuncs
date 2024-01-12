
#include <proto/exec.h>
#include "SafeOpenLibrary.h"

struct Library *TextFieldBase;

void OpenTextEditorBase() __attribute__((constructor));
void CloseTextEditorBase() __attribute__((destructor));

void OpenTextEditorBase()
{
	TextFieldBase = SafeOpenLibrary("Gadgets/texteditor.gadget");
}

void CloseTextEditorBase()
{
	CloseLibrary(TextFieldBase);
}
