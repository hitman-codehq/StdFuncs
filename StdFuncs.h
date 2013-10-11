
#ifndef STDFUNCS_H
#define STDFUNCS_H

#ifdef __amigaos4__

#include <proto/exec.h>
#include <proto/dos.h>

#elif defined(__linux__)

#include <sys/param.h>

#define TRUE 1
#define FALSE 0

typedef void * APTR;
typedef int BOOL;
typedef long LONG;
typedef unsigned char UBYTE;
typedef unsigned long ULONG;

#define stricmp(String1, String2) strcasecmp(String1, String2)
#define strnicmp(String1, String2, Length) strncasecmp(String1, String2, Length)

#else /* ! __linux__ */

/* Enable newer functionality such as mouse wheel handling */

#define _WIN32_WINNT 0x0400

#include <windows.h>
#include <signal.h>

typedef void * APTR;
typedef unsigned char UBYTE;

#endif /* ! __linux__ */

#if !defined(__linux__)

/* We used to define this as max() but bizarrely Qt undefines it when you include certain headers! */
/* So now we use MAX() instead and for Linux we use the one in sys/param.h instead */

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#endif /* !defined(__linux) */

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
#define KErrCorrupt			-20
#define KErrWrite			-23
#define KErrEof				-25

/* Useful macros for the WinMain() and main() functions, enabling them to be used */
/* without #ifdefs in portable code */

#ifdef WIN32

#define TFMain() TInt WINAPI WinMain(HINSTANCE /*a_hInstance*/, HINSTANCE /*a_hPrevInstance*/, char *a_pcCmdParam, TInt /*a_iCmdShow*/)
#define TFArgs a_pcCmdParam

#else /* ! WIN32 */

#define TFMain() TInt main(TInt a_iArgC, const char *a_ppcArgV[])
#define TFArgs a_ppcArgV, a_iArgC

#endif /* ! WIN32 */

/* Useful Amiga OS return value constants for main() */

#ifndef __amigaos4__

#define RETURN_OK 0
#define RETURN_ERROR 10

#endif /* ! __amigaos4__ */

/* Useful Windows constants for OK and cancel buttons and maximum path */

#ifndef WIN32

#define IDOK 1
#define IDCANCEL 2
#define IDYES 6
#define IDNO 7
#define MAX_PATH 260

typedef unsigned long COLORREF;

#define RGB(r, g, b) (r | (g << 8) | (b << 16))

#endif /* ! WIN32 */

/* Standard keycodes that are passed to client code through CWindow::OfferKeyEvent().  Keys */
/* are grouped into sets with a similar function to allow client code to check key values */
/* against ranges of keys and these sets/ranges are guaranteed not to change (although numeric */
/* values within the ranges may).  Thus it is safe to perform check such as: */
/* */
/* if ((Key >= STD_KEY_UP) && (Key <= STD_KEY_PGDN)) */
/* */
/* To check whether a key is a movement key */

#define STD_KEY_SHIFT 256
#define STD_KEY_CONTROL 257
#define STD_KEY_ALT 258
#define STD_KEY_BACKSPACE 259
#define STD_KEY_ENTER 260
#define STD_KEY_TAB 261
#define STD_KEY_DELETE 262
#define STD_KEY_UP 263
#define STD_KEY_DOWN 264
#define STD_KEY_LEFT 265
#define STD_KEY_RIGHT 266
#define STD_KEY_HOME 267
#define STD_KEY_END 268
#define STD_KEY_PGUP 269
#define STD_KEY_PGDN 270
#define STD_KEY_ESC 271
#define STD_KEY_F1 272
#define STD_KEY_F2 273
#define STD_KEY_F3 274
#define STD_KEY_F4 275
#define STD_KEY_F5 276
#define STD_KEY_F6 277
#define STD_KEY_F7 278
#define STD_KEY_F8 279
#define STD_KEY_F9 280
#define STD_KEY_F10 281
#define STD_KEY_F11 282
#define STD_KEY_F12 283

/* Structure used for mapping standard keycodes onto native keycodes */

struct SKeyMapping
{
	TInt	m_iStdKey;			/* Standard keycode */
	TInt	m_iNativeKey;		/* Native keycode */
};

#include "Utils.h"

#endif /* ! STDFUNCS_H */
