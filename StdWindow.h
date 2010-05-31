
#ifndef STDWINDOW_H
#define STDWINDOW_H

class CWindow
{
// TODO: CAW - Use accessors?
public:

#ifdef __amigaos4__

	struct Window	*m_poWindow;	/* Ptr to underlying Intuition window */

#else /* ! __amigaos4__ */

	HWND			m_poWindow;		/* Ptr to underlying Windows window */
	HDC				m_poDC;			// TODO: CAW - What about initialising these?
	PAINTSTRUCT		m_oPaintStruct;	// TODO: CAW

#endif /* ! __amigaos4__ */

public:

	~CWindow();

	TInt Open(const char *a_pccTitle);

	void Close();

	virtual void Draw() = 0;

	// TODO: CAW - Check type
	void DrawNow();

	// TODO: CAW - Shouldn't be pure
	virtual void OfferKeyEvent(int a_iKey) = 0;

#ifdef __amigaos4__

	ULONG GetSignal();

	struct Window *GetWindow();

#endif /* __amigaos4__ */

};

#endif /* ! STDWINDOW_H */
