
#ifndef STDREACTION_H
#define STDREACTION_H

/* A small header to avoid having to repeat this sequence in each framework */
/* implementation file */

#ifdef __amigaos4__

#define ALL_REACTION_CLASSES
#define ALL_REACTION_MACROS

#include <proto/intuition.h>
#include <reaction/reaction.h>

#endif /* __amigaos4__ */

#endif /* ! STDREACTION_H */
