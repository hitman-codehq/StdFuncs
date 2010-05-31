
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

	m_poWindow = a_poWindow;
}

/* Written: Sunday 31-May-2010 3:38 pm */

TInt RFont::Open()
{
	TInt RetVal;

#ifdef __amigaos4__

	ASSERTM(m_poWindow, "RFont::Open() => Window handle not set");

	/* RFont::Open() cannow fail on the Amiga */

	RetVal = KErrNone;

	/* Determine the baseline & height of the font from the window */

	m_iBaseline = a_poWindow->m_poWindow->IFont->tf_Baseline;
	m_iHeight = a_poWindow->m_poWindow->IFont->tf_YSize;

#else /*  ! __amigaos4__ */

	TEXTMETRIC TextMetric;

	ASSERTM(m_poWindow->m_poDC, "RFont::Open() => Function should only be called from CWindow::Draw");

	/* Determine the height of the font from the device context */

	if (GetTextMetrics(m_poWindow->m_poDC, &TextMetric))
	{
		RetVal = KErrNone;

		m_iHeight = TextMetric.tmHeight;
	}
	else
	{
		RetVal = KErrGeneral;
	}

#endif /*  ! __amigaos4__ */

	return(RetVal);
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

	TextOut(m_poWindow->m_poDC, 0, (a_iY * m_iHeight), a_pcText, a_iLength);

#endif /* ! __amigaos4__ */

}

/* Written: Sunday 31-May-2010 2:36 pm */

TInt RFont::Height()
{
	return(m_iHeight);
}
