
#include <proto/exec.h>
#include <stdio.h>
#include <stdlib.h>

/* The texteditor.gadget started off as an external contribution and does not follow the normal Amiga OS */
/* versioning scheme */
#define MINIMUM_VERSION 15

static const char g_libraryName[] = "Gadgets/texteditor.gadget";

struct Library *TextFieldBase;

void OpenTextEditorBase() __attribute__((constructor));
void CloseTextEditorBase() __attribute__((destructor));

void OpenTextEditorBase()
{
	/* Manually open the gadget, rather than calling SafeOpenLibrary() */
	if ((TextFieldBase = OpenLibrary(g_libraryName, MINIMUM_VERSION)) == NULL)
	{
		printf("Error: Unable to load library \"%s\" version %d\n", g_libraryName, MINIMUM_VERSION);
		exit(10);
	}
}

void CloseTextEditorBase()
{
	CloseLibrary(TextFieldBase);
}
