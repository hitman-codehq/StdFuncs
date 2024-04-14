
#include "StdFuncs.h"
#include "OS4Support.h"
#include <clib/alib_protos.h>

/**
 * Allocates a system object.
 * An convenience function allowing easy allocation of ports, semaphores etc.
 *
 * @date	Saturday 11-Apr-2020 7:52 am, Code HQ Bergmannstrasse
 * @param	a_type			The type of object to be allocated, such as ASOT_PORT
 * @return	The allocated and initialised object, if successful, otherwise NULL
 */

APTR AllocSysObject(ULONG a_type, const struct TagItem *a_tags)
{
	APTR retVal;

	if (a_type == ASOT_PORT)
	{
		/* Ensure that the port is in public memory so it is accessible between tasks and in interrupts */
		if ((retVal = AllocMem(sizeof(struct MsgPort), (MEMF_CLEAR | MEMF_PUBLIC))) != nullptr)
		{
			struct MsgPort *msgPort = static_cast<MsgPort *>(retVal);

			/* Iterate through the tags passed in and check for the name and priority tags */
			while (a_tags->ti_Tag != TAG_DONE)
			{
				if (a_tags->ti_Tag == ASOPORT_Name)
				{
					msgPort->mp_Node.ln_Name = reinterpret_cast<char *>(a_tags->ti_Data);
				}
				else if (a_tags->ti_Tag == ASOPORT_Pri)
				{
					msgPort->mp_Node.ln_Pri = static_cast<BYTE>(a_tags->ti_Data);
				}

				a_tags = (a_tags + 1);
			}

			/* And initialise the other mandatory fields in the port */
			msgPort->mp_Node.ln_Type = NT_MSGPORT;
			msgPort->mp_SigBit = AllocSignal(-1);
			msgPort->mp_SigTask = FindTask(nullptr);
			NewList(&msgPort->mp_MsgList);

			/* If a name was passed in then add the port to the public message list. It is the responsibility of */
			/* the caller to remove the port from the list before freeing it */
			if (msgPort->mp_Node.ln_Name)
			{
				AddPort(msgPort);
			}
		}
	}
	else if (a_type == ASOT_SEMAPHORE)
	{
		/* Ensure that the port is in public memory so it is accessible between tasks and in interrupts */
		if ((retVal = AllocMem(sizeof(struct SignalSemaphore), (MEMF_CLEAR | MEMF_PUBLIC))) != nullptr)
		{
			InitSemaphore(static_cast<SignalSemaphore *>(retVal));
		}
	}
	else
	{
		retVal = nullptr;
	}

	return retVal;
}

/**
 * Convenience wrapper around AllocSysObject.
 * This wrapper simply takes the pointer to the list of tags passed in on the stack and calls AllocSysObject
 * using them.
 *
 * @date	Monday 15-Apr-2024 6:02 am, Code HQ Tokyo Tsukuda
 * @param	a_type			The type of object to be allocated, such as ASOT_PORT
 * @param	a_tag			The first tag of the stack-based list of tags to use
 * @return	The return value from AllocSysObject
 */

APTR AllocSysObjectTags(ULONG a_type, Tag a_tag, ...)
{
	return AllocSysObject(a_type, (struct TagItem *) &a_tag);
}

/**
 * Converts a BCPL string to a C string.
 * Safely copies a BSTR into a user supplied buffer, NULL terminating it at the appropriate length, and checking
 * that the string is able to fit into the target buffer.
 *
 * @date	Wednesday 02-Jun-2021 6:53 am, Code HQ Bergmannstrasse
 * @param	a_bstring		Pointer to the BSTR to be copied
 * @param	a_dest			Pointer to the target buffer
 * @param	a_size			Size of the target buffer, in bytes
 * @return	The length of the original BSTR
 */

ULONG CopyStringBSTRToC(BSTR a_bstring, STRPTR a_dest, ULONG a_size)
{
	const char *source = static_cast<const char *>(BADDR(a_bstring));
	ULONG sourceLength = (source != nullptr) ? *source++ : 0;

	/* Only try to copy anything if there is enough space in the target buffer */
	if (a_size > 0)
	{
		int destLength = (sourceLength < (a_size - 1)) ? sourceLength : a_size - 1;

		/* And if a target buffer was actually passed in */
		if (a_dest != nullptr)
		{
			for (int index = 0; index < destLength; ++index)
			{
				*a_dest++ = *source++;
			}

			*a_dest = '\0';
		}
	}

	/* Always return the length of the BSTR, regardless of how many bytes were copied */
	return sourceLength;
}

/**
 * Converts a date stamp to Amiga time.
 * Takes a DateStamp structure, which consists of days and minutes since 01.01.1978, and converts it into
 * an integer representing the number of seconds since that time.
 *
 * @date	Saturday 27-Feb-2021 3:00 pm, Code HQ Bergmannstrasse
 * @param	a_dateStamp		A pointer to the date stamp to be converted
 * @return	The value of the date stamp in Amiga time
 */

ULONG DateStampToSeconds(const struct DateStamp *a_dateStamp)
{
	ULONG retVal;

	retVal = (a_dateStamp->ds_Days * SECONDS_PER_DAY);
	retVal += (a_dateStamp->ds_Minute * 60);

	return retVal;
}

/**
 * Frees a system object.
 * A convenience function allowing the deallocation of system objects allocated with AllocSysObject().
 *
 * @date	Saturday 25-Apr-2020 12:58 pm, Code HQ Bergmannstrasse
 * @param	a_type			The type of object to be deallocated, such as ASOT_PORT
 * @param	a_object		A pointer to the object to be deallocated
 */

void FreeSysObject(ULONG a_type, APTR a_object)
{
	if (a_type == ASOT_PORT)
	{
		struct MsgPort *msgPort = static_cast<MsgPort *>(a_object);

		/* Free the signal bit if it has been allocated. Values from 0 - 31 are valid and -1 means no signal. */
		/* Due to the ancient and broken Amiga API (mb_SigBit is UBYTE but AllocSignal() returns a BYTE), we */
		/* have to use a cast with -1, at least on modern compilers */
		if (msgPort->mp_SigBit != static_cast<UBYTE>(-1))
		{
			FreeSignal(msgPort->mp_SigBit);
		}

		FreeMem(a_object, sizeof(struct MsgPort));
	}
	else if (a_type == ASOT_SEMAPHORE)
	{
		FreeMem(a_object, sizeof(struct SignalSemaphore));
	}
}

/**
 * Sets one or more attributes and refreshes gadget.
 * This is a convenience function that will perform a SetGadgetAttrsA() using the parameters given, and if the
 * target gadget indicates that it needs to be refreshed, it will also perform a refresh of that gadget.
 *
 * @date	Wednesday 14-Jul-2021 8:20 am, Code HQ Bergmannstrasse
 * @param	a_gadget		Pointer to the BOOPSI gadget object
 * @param	a_window		Pointer to the window in which the gadget resides, or NULL
 * @param	a_requester		Pointer to the requester in which the gadget resides, or NULL
 * @param	a_taglist		Pointer to the taglist of attributes to be applied to the gadget
 */

void RefreshSetGadgetAttrsA(struct Gadget *a_gadget, struct Window *a_window, struct Requester *a_requester,
	struct TagItem *a_taglist)
{
	if (SetGadgetAttrsA(a_gadget, a_window, a_requester, a_taglist) != 0)
	{
		RefreshGList(a_gadget, a_window, a_requester, 1);
	}
}

/**
 * Convenience wrapper around RefreshSetGadgetAttrs.
 * This wrapper simply takes the pointer to the list of tags passed in on the stack and calls RefreshSetGadgetAttrs
 * using them.
 *
 * @date	Monday 15-Apr-2024 6:10 am, Code HQ Tokyo Tsukuda
 * @param	a_gadget		Pointer to the BOOPSI gadget object
 * @param	a_window		Pointer to the window in which the gadget resides, or NULL
 * @param	a_requester		Pointer to the requester in which the gadget resides, or NULL
 * @param	a_tag			The first tag of the stack-based list of tags to use
 * @return	The return value from RefreshSetGadgetAttrs
 */

void RefreshSetGadgetAttrs(struct Gadget *a_gadget, struct Window *a_window, struct Requester *a_requester,
	Tag a_tag, ...)
{
	RefreshSetGadgetAttrsA(a_gadget, a_window, a_requester, (struct TagItem *) &a_tag);
}
