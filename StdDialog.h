
#ifndef STDDIALOG_H
#define STDDIALOG_H

/* Forward declaration to reduce the # of includes required */

class CWindow;

/* This is the base class for all platform specific dialogs */

class CDialog
{
protected:

	char	*m_pcTextBuffer;		/* Scratch buffer containing last obtained text */
	TInt	m_iTextBufferLength;	/* Length of scratch buffer */

public:

	HWND	m_poWindow;			/* Ptr to underlying Windows window for dialog */
	CWindow	*m_poParentWindow;	/* Ptr to window in which dialog is opened */

public:

	CDialog(CWindow *a_poParentWindow)
	{
		m_poParentWindow = a_poParentWindow;
	}

	~CDialog()
	{
		delete [] m_pcTextBuffer;
	}

	virtual TBool HandleCommand(TInt /*a_iCommand*/)
	{
		return(EFalse);
	}

protected:

	CDialog() { }

	TBool GetGadgetText(TInt a_iGadgetID);

	virtual TInt Open(TInt a_ResourceID);
};

#endif /* ! STDDIALOG_H */
