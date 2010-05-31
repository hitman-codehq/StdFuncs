
#include "StdFuncs.h"
#include "StdFont.h"
#include "StdWindow.h"

#ifdef __amigaos4__

#include <proto/graphics.h>

#endif /* __amigaos4__ */

/* Written: Sunday 31-May-2010 1:41 pm */

RFont::RFont(CWindow *a_poWindow)
{
	ASSERTM(a_poWindow, "RFont::RFont() => Window handle must not be NULL");

#ifdef __amigaos4__

	m_iBaseline = a_poWindow->m_poWindow->IFont->tf_Baseline;
	m_iHeight = a_poWindow->m_poWindow->IFont->tf_YSize;

#endif /*  __amigaos4__ */

	m_poWindow = a_poWindow;
}

/* Written: Sunday 09-May-2010 6:57 pm */

void RFont::DrawText(const char *a_pcText, TInt a_iLength, TInt a_iY)
{
	ASSERTM(m_poWindow, "RFont::DrawText() => Window handle not set");

#ifdef __amigaos4__

	/* Move to the position at which to print, taking into account the left and top border sizes, */
	/* the height of the current font and the baseline of the font, given that IGraphics->Text() */
	/* routine prints at the baseline position, not the top of the font */

	IGraphics->Move(m_poWindow->m_poWindow->RPort, m_poWindow->m_poWindow->BorderLeft,
		(m_poWindow->m_poWindow->BorderTop + (a_iY * m_iHeight) + m_iBaseline));

	/* Set the background and foreground pens, in case they have been changed since the last call */

	IGraphics->SetAPen(m_poWindow->m_poWindow->RPort, 1);
	IGraphics->SetBPen(m_poWindow->m_poWindow->RPort, 0);

	/* And draw the text passed in */

	IGraphics->Text(m_poWindow->m_poWindow->RPort, a_pcText, a_iLength);

#else /* ! __amigaos4__ */

	// TODO: CAW - Do this in open so it can be cached?
	TInt Height;
	TEXTMETRIC TextMetric;

	if (GetTextMetrics(m_poWindow->m_poDC, &TextMetric))
	{
		Height = TextMetric.tmHeight;

		// TODO: CAW - What about checking m_poDC is ok?
		TextOut(m_poWindow->m_poDC, 0, (a_iY * Height), a_pcText, a_iLength);
	}

#endif /* ! __amigaos4__ */

}

/* Written: Sunday 31-May-2010 2:36 pm */

TInt RFont::Height()
{

#ifdef __amigaos4__

	return(m_iHeight);

#else /* ! __amigaos4__ */

#endif /* ! __amigaos4__ */

}
