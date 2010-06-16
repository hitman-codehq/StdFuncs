
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
#define KErrNoMemory		-4
#define KErrAlreadyExists	-11
#define KErrPathNotFound	-12
#define KErrInUse			-14
#define KErrCompletion		-17
#define KErrWrite			-23
#define KErrEof				-25

#ifndef __amigaos4__

#define RETURN_OK 0
#define RETURN_ERROR 10

#endif /* ! __amigaos4__ */

/* Standard key scancodes for Amiga OS and Windows */

#ifdef __amigaos4__

#define STD_KEY_ENTER 0x0d
#define STD_KEY_UP 0x4c
#define STD_KEY_DOWN 0x4d
#define STD_KEY_LEFT 0x4f
#define STD_KEY_RIGHT 0x4e
#define STD_KEY_HOME 0x70
#define STD_KEY_END 0x71
#define STD_KEY_PGUP 0x48
#define STD_KEY_PGDN 0x49

#else /* ! __amigaos4__ */

#define STD_KEY_BACKSPACE VK_BACK
#define STD_KEY_ENTER VK_RETURN
#define STD_KEY_UP VK_UP
#define STD_KEY_DOWN VK_DOWN
#define STD_KEY_LEFT VK_LEFT
#define STD_KEY_RIGHT VK_RIGHT
#define STD_KEY_HOME VK_HOME
#define STD_KEY_END VK_END
#define STD_KEY_PGUP VK_PRIOR
#define STD_KEY_PGDN VK_NEXT
#define STD_KEY_DELETE VK_DELETE

#endif /* ! __amigaos4__ */

#include "Utils.h"

#endif /* ! STDFUNCS_H */
