
#ifndef STDFUNCS_H
#define STDFUNCS_H

#ifdef __amigaos4__

#include <proto/exec.h>
#include <proto/dos.h>

#else /* ! __amigaos4__ */

#include <windows.h>
#include <signal.h>

typedef void * APTR;
typedef unsigned char UBYTE;

#endif /* ! __amigaos4__ */

#include "MungWall.h"

/* Fundamental type declarations */

typedef signed int TInt;
typedef unsigned int TUint;
typedef int TBool;

/* Values for use with TBool */

#define ETrue 1
#define EFalse 0

/* System wide error codes that can be returned by all StdFuncs and StdFuncs related classes. */
/* Any time that an error is added here, ensure that handling for is added to Utils::Error() */

#define KErrNone			0
#define KErrNotFound		-1
#define KErrGeneral			-2
#define KErrCancel			-3
#define KErrNoMemory		-4
#define KErrAlreadyExists	-11
#define KErrPathNotFound	-12
#define KErrInUse			-14
#define KErrCompletion		-17
#define KErrWrite			-23
#define KErrEof				-25

/* Useful Amiga OS return value constants for main() */

#ifndef __amigaos4__

#define RETURN_OK 0
#define RETURN_ERROR 10

#endif /* ! __amigaos4__ */

/* Useful Win32 constants for OK and cancel buttons */

#ifndef WIN32

#define IDOK 1
#define IDCANCEL 2

#endif /* WIN32 */

/* Standard keycodes that are passed to client code through CWindow::OfferKeyEvent() */

#define STD_KEY_SHIFT 128
#define STD_KEY_CONTROL 129
#define STD_KEY_BACKSPACE 130
#define STD_KEY_ENTER 131
#define STD_KEY_UP 132
#define STD_KEY_DOWN 133
#define STD_KEY_LEFT 134
#define STD_KEY_RIGHT 135
#define STD_KEY_HOME 136
#define STD_KEY_END 137
#define STD_KEY_PGUP 138
#define STD_KEY_PGDN 139
#define STD_KEY_DELETE 140
#define STD_KEY_TAB 141
#define STD_KEY_ESC 142
#define STD_KEY_F3 143

/* Structure used for mapping standard keycodes onto native keycodes */

struct SKeyMapping
{
	int	m_iStdKey;			/* Standard keycode */
	int	m_iNativeKey;		/* Native keycode */
};

#include "Utils.h"

#endif /* ! STDFUNCS_H */
