
#include <proto/exec.h>
#include <stdio.h>
#include <stdlib.h>

#define MINIMUM_VERSION 15

/**
 * Opens the requested library and bails out if it fails.
 * This support function will open the requested library and, if it is not able to be opened, will print an
 * error message and shut down the calling application.
 *
 * @date	Friday 20-Aug-2021 6:25 pm, By the Zwinger pond, Dresden
 * @param	a_libraryName	The name of the library to be opened
 * @return	A pointer the opened library, if successful
 */

struct Library *SafeOpenLibrary(const char *a_libraryName)
{
	struct Library *retVal;

	if ((retVal = OpenLibrary(a_libraryName, MINIMUM_VERSION)) == NULL)
	{
		printf("Error: Unable to load library \"%s\" version %d\n", a_libraryName, MINIMUM_VERSION);
		exit(10);
	}

	return retVal;
}
