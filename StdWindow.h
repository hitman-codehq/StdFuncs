
#ifndef STDWINDOW_H
#define STDWINDOW_H

class CWindow
{
private:

#ifdef __amigaos4__

	struct Window	*m_poWindow;	/* Ptr to underlying Intuition window */

#else /* ! __amigaos4__ */

	HWND	m_poWindow;				/* Ptr to underlying Windows window */

#endif /* ! __amigaos4__ */

public:

	~CWindow();

	int Open(const char *a_pccTitle);

	void Close();

#ifdef __amigaos4__

	ULONG GetSignal();

#endif /* __amigaos4__ */

};

#endif /* ! STDWINDOW_H */
