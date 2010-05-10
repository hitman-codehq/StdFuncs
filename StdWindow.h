
#ifndef STDWINDOW_H
#define STDWINDOW_H

class CWindow
{
private:

#ifdef __amigaos4__

	struct Window	*m_poWindow;	/* Ptr to underlying Intuition window */

#else /* ! __amigaos4__ */

// TODO: CAW - Use accessors?
public:

	HWND			m_poWindow;		/* Ptr to underlying Windows window */
	HDC				m_poDC;			// TODO: CAW - What about initialising these?
	PAINTSTRUCT		m_oPaintStruct;	// TODO: CAW

#endif /* ! __amigaos4__ */

public:

	~CWindow();

	TInt Open(const char *a_pccTitle);

	void Close();

	virtual void Draw() = 0;

#ifdef __amigaos4__

	ULONG GetSignal();

	struct Window *GetWindow();

#endif /* __amigaos4__ */

};

#endif /* ! STDWINDOW_H */
