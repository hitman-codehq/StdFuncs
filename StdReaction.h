
#ifndef STDREACTION_H
#define STDREACTION_H

/** @file */

/* A small header to avoid having to repeat this sequence in each framework */
/* implementation file */

#ifdef __amigaos__

#define ALL_REACTION_CLASSES
#define ALL_REACTION_MACROS

#include <proto/intuition.h>
#include <reaction/reaction.h>
#include <reaction/reaction_macros.h>

#endif /* __amigaos__ */

#endif /* ! STDREACTION_H */
