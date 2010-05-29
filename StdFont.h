
#ifndef STDFONT_H
#define STDFONT_H

/* Forward declaration to reduce the # of includes required */

class CWindow;

class RFont
{
private:

	// TODO: CAW - Make reference?
	CWindow		*m_poWindow;	/* Ptr to window into which to render */

public:

	// TODO: CAW - Move?
	RFont(CWindow *a_poWindow)
	{
		m_poWindow = a_poWindow;
	}

	void DrawText(const char *a_pcText, TInt a_iLength, TInt a_iY);
};

#endif /* ! STDFONT_H */
