
#ifndef STDDIALOG_H
#define STDDIALOG_H

#include "StdWindow.h"

/* Event types that can be sent to CDialog::HandleEvent() */

enum TStdEventType
{
	EStdEventChange = 0x0300		/* Gadget contents have changed */
};

/* This is the base class for all platform specific dialog boxes */

class CDialog : public CWindow
{
protected:

	char	*m_pcTextBuffer;		/* Scratch buffer containing last obtained text */
	TInt	m_iTextBufferLength;	/* Length of scratch buffer */

public:

#ifdef WIN32

	HWND	m_poWindow;				/* Ptr to underlying Windows window for dialog */

#endif /* WIN32 */
	
	CWindow	*m_poParentWindow;		/* Ptr to window in which dialog is opened */

public:

	CDialog(CWindow *a_poParentWindow)
	{
		m_poParentWindow = a_poParentWindow;
	}

	~CDialog()
	{
		delete [] m_pcTextBuffer;
	}

	/* Functions can be implemented by client software */

	virtual void HandleEvent(enum TStdEventType /*a_eEventID*/, TInt /*a_iGadgetID*/) { }

	virtual void InitDialog() { }

protected:

	CDialog() { }

	virtual TInt Open(TInt a_ResourceID);

	void EnableGadget(TInt a_iGadgetID, TBool a_bEnable);

	TInt GetGadgetInt(TInt a_iGadgetID);

	TInt GetGadgetText(TInt a_iGadgetID, TBool a_bGetText);

	void CheckGadget(TInt a_iGadgetID);

	TBool IsGadgetChecked(TInt a_iGadgetID);

	void SetGadgetText(TInt a_iGadgetID, const char *a_pccText);
};

#endif /* ! STDDIALOG_H */
