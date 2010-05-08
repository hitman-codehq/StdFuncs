
#ifndef STDFUNCS_H
#define STDFUNCS_H

#ifdef __amigaos4__

#include <proto/exec.h>
#include <proto/dos.h>

#else /* ! __amigaos4__ */

#include <windows.h>
#include <signal.h>

// TODO: CAW - Check type on Amiga
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

#ifndef __amigaos4__

#define RETURN_OK 0
#define RETURN_ERROR 10

#endif /* ! __amigaos4__ */

#include "Utils.h"

#endif /* ! STDFUNCS_H */
