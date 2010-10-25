
#ifndef STDDIALOG_H
#define STDDIALOG_H

#include "StdWindow.h"

/* Event types that can be sent to CDialog::HandleEvent() */

enum TStdEventType
{
	EStdEventChange = 0x0300		/* Gadget contents have changed */
};

#ifdef __amigaos4__

/* Each instance of this structure represents an Amiga OS gadget mapping */

struct SGadgetMapping
{
	APTR	m_poGadget;				/* Ptr to BOOPSI gadget */
	TInt	m_iID;					/* Integer ID of the gadget */
};

#endif /* __amigaos4__ */

/* This is the base class for all platform specific dialog boxes */

class CDialog : public CWindow
{
protected:

	char					*m_pcTextBuffer;		/* Scratch buffer containing last obtained text */
	TInt					m_iTextBufferLength;	/* Length of scratch buffer */
	struct SGadgetMapping	*m_poGadgetMappings;	/* Array of gadget ID -> APTR mappings */
	TInt					m_iNumGadgetMappings;	/* # of entries in m_poGadgetMappings */

public:

#ifdef WIN32

	HWND					m_poWindow;				/* Ptr to underlying Windows window for dialog */

#endif /* WIN32 */
	
	CWindow					*m_poParentWindow;		/* Ptr to window in which dialog is opened */

public:

	CDialog(RApplication *a_poApplication, CWindow *a_poParentWindow) : CWindow(a_poApplication)
	{
		m_poParentWindow = a_poParentWindow;
	}

	~CDialog()
	{
		delete [] m_pcTextBuffer;
		delete [] m_poGadgetMappings;
	}

	/* Functions can be implemented by client software */

	virtual void HandleEvent(enum TStdEventType /*a_eEventID*/, TInt /*a_iGadgetID*/) { }

	virtual void InitDialog() { }

	virtual void OfferKeyEvent(TInt /*a_iKey*/, TBool /*a_iKeyDown*/);

protected:

	CDialog() { }

	virtual TInt Open(TInt a_ResourceID);

	void Close();

	void EnableGadget(TInt a_iGadgetID, TBool a_bEnable);

#ifdef __amigaos4__

	APTR GetBOOPSIGadget(TInt a_iGadgetID);

#endif /* __amigaos4__ */

	TInt GetGadgetInt(TInt a_iGadgetID);

	TInt GetGadgetText(TInt a_iGadgetID, TBool a_bGetText);

	void CheckGadget(TInt a_iGadgetID);

	TBool IsGadgetChecked(TInt a_iGadgetID);

	void SetGadgetFocus(TInt a_iGadgetID);

	void SetGadgetText(TInt a_iGadgetID, const char *a_pccText);
};

#endif /* ! STDDIALOG_H */
