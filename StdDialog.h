
#ifndef STDDIALOG_H
#define STDDIALOG_H

/* Forward declaration to reduce the # of includes required */

class CWindow;

/* Event types that can be sent to CDialog::HandleEvent() */

enum TStdEventType
{
	EStdEventChange = 0x0300		/* Gadget contents have changed */
};

/* This is the base class for all platform specific dialogs */

class CDialog
{
protected:

	char	*m_pcTextBuffer;		/* Scratch buffer containing last obtained text */
	TInt	m_iTextBufferLength;	/* Length of scratch buffer */

public:

	HWND	m_poWindow;				/* Ptr to underlying Windows window for dialog */
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

	virtual void InitDialog() { }

	virtual TBool HandleCommand(TInt /*a_iCommand*/)
	{
		return(EFalse);
	}

	virtual void HandleEvent(enum TStdEventType /*a_eEventID*/, TInt /*a_iGadgetID*/) { }

protected:

	CDialog() { }

	virtual TInt Open(TInt a_ResourceID);

	void EnableGadget(TInt a_iGadgetID, TBool a_bEnable);

	TInt GetGadgetText(TInt a_iGadgetID, TBool a_bGetText);

	void SetGadgetText(TInt a_iGadgetID, const char *a_pccText);
};

#endif /* ! STDDIALOG_H */
