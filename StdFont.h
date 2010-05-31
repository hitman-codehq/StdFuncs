
#ifndef STDFONT_H
#define STDFONT_H

/* Forward declaration to reduce the # of includes required */

class CWindow;

class RFont
{
private:

#ifdef __amigaos4__

	int			m_iBaseline;	/* Distance from the top of char to baseline */
	int			m_iHeight;		/* Height of the font in pixels */

#endif /* __amigaos4__ */

	CWindow		*m_poWindow;	/* Ptr to window into which to render */

public:

	RFont(CWindow *a_poWindow);

	void DrawText(const char *a_pcText, TInt a_iLength, TInt a_iY);

	TInt Height();
};

#endif /* ! STDFONT_H */
