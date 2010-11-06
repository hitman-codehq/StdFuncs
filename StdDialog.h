
#ifndef STDDIALOG_H
#define STDDIALOG_H

#include "StdWindow.h"

/* Forward declaration to avoid circular dependency */

class CDialog;

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

/* Mixin class for notifying client code that a dialog event has occurred */

class MDialogObserver
{
public:

	virtual void DialogClosed(CDialog *a_poDialog, TInt a_iGadgetID) = 0;
};

/* This is the base class for all platform specific dialog boxes */

class CDialog : public CWindow
{
private:

	MDialogObserver			*m_poDialogObserver;	/* Ptr to dialog's observer to notify of events */

protected:

	char					*m_pcTextBuffer;		/* Scratch buffer containing last obtained text */
	TInt					m_iTextBufferLength;	/* Length of scratch buffer */
	struct SGadgetMapping	*m_poGadgetMappings;	/* Array of gadget ID -> APTR mappings */
	TInt					m_iNumGadgetMappings;	/* # of entries in m_poGadgetMappings */

public:

	CDialog(RApplication *a_poApplication, MDialogObserver *a_poDialogObserver = NULL)
		: CWindow(a_poApplication), m_poDialogObserver(a_poDialogObserver)
	{
		/* Dialogs don't have their backgrounds filled */

		EnableFillBackground(EFalse);
	}

	~CDialog()
	{
		delete [] m_pcTextBuffer;

#ifdef __amigaos4__

		delete [] m_poGadgetMappings;

#endif /* __amigaos4__ */

	}

	/* Functions can be implemented by client software */

	virtual void HandleEvent(enum TStdEventType /*a_eEventID*/, TInt /*a_iGadgetID*/) { }

	virtual void InitDialog() { }

	virtual void OfferKeyEvent(TInt /*a_iKey*/, TBool /*a_iKeyDown*/);

protected:

	virtual TInt Open(TInt a_ResourceID);

	virtual void Close()
	{
		CWindow::Close();
	}

	void Close(TInt a_iGadgetID);

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

private:

	CDialog() { }
};

#endif /* ! STDDIALOG_H */
