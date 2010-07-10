
#ifndef STDFONT_H
#define STDFONT_H

/* Forward declaration to reduce the # of includes required */

class CWindow;

class RFont
{
private:

#ifdef __amigaos4__

	TInt		m_iBaseline;	/* Distance from the top of char to baseline */

#else /* ! __amigaos4__ */

	HFONT		m_poFont;		/* Win32 font with which to render */
	HFONT		m_poOldFont;	/* Win32 font previously selected into window */

#endif /* ! __amigaos4__ */

	TInt		m_iHeight;		/* Height of the font in pixels */
	CWindow		*m_poWindow;	/* Ptr to window into which to render */

public:

	RFont(CWindow *a_poWindow);

	TInt Open();

	void Close();

	void DrawCursor(const char *a_pcText, TInt a_iX, TInt a_iY);

	void DrawText(const char *a_pcText, TInt a_iLength, TInt a_iY, TBool a_iHighlight);

	TInt Height();
};

#endif /* ! STDFONT_H */
