
#ifndef STDWINDOW_H
#define STDWINDOW_H

/* Forward declaration to reduce the # of includes required */

class RApplication;

class CWindow
{
protected:

	TInt			m_iInnerWidth;		/* Width of window, minus left and right borders */
	TInt			m_iInnerHeight;		/* Height of window, minus top and bottom borders */
	RApplication	*m_poApplication;	/* Ptr to application that owns this window */

public:

#ifdef __amigaos4__

	struct Window	*m_poWindow;	/* Ptr to underlying Intuition window */

#else /* ! __amigaos4__ */

	HWND			m_poWindow;		/* Ptr to underlying Windows window */
	HDC				m_poDC;			/* Device context and paint structure into which to */
	PAINTSTRUCT		m_oPaintStruct;	/* render;  valid only during calls to CWindow::Draw() */

#endif /* ! __amigaos4__ */

public:

	~CWindow();

	TInt Open(const char *a_pccTitle, const char *a_pccPubScreenName);

	void Close();

	virtual void Draw() = 0;

	void DrawNow();

	virtual void HandleCommand(TInt /*a_iCommand*/) { }

	virtual void OfferKeyEvent(TInt /*a_iKey*/, TBool /*a_iKeyDown*/) { }

	void SetApplication(RApplication *a_poApplication)
	{
		m_poApplication = a_poApplication;
	}

#ifdef __amigaos4__

	ULONG GetSignal();

	struct Window *GetWindow();

#endif /* __amigaos4__ */

};

#endif /* ! STDWINDOW_H */
