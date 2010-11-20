
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
	COLORREF	m_oBackground;	/* Background colour and text colours at the */
	COLORREF	m_oText;		/* time that RFont.Open() was callsed */

#endif /* ! __amigaos4__ */

	TBool		m_iHighlight;	/* ETrue if text will be drawn highlighted, else EFalse */
	TInt		m_iWidth;		/* Width of a character in pixels.  Valid only for monospace fonts */
	TInt		m_iHeight;		/* Height of a character in pixels */
	CWindow		*m_poWindow;	/* Ptr to window into which to render */

public:

	RFont(CWindow *a_poWindow);

	TInt Open();

	void Close();

	void DrawCursor(const char *a_pcText, TInt a_iX, TInt a_iY, TBool a_iDrawCharacter);

	void DrawText(const char *a_pcText, TInt a_iLength, TInt a_iY);

	TInt Width()
	{
		return(m_iWidth);
	}

	TInt Height()
	{
		return(m_iHeight);
	}

	void SetHighlight(TBool a_iHighlight);
};

#endif /* ! STDFONT_H */
