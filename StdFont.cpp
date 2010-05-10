
#include "StdFuncs.h"
#include "StdFont.h"
#include "StdWindow.h"

/* Written: Sunday 09-May-2010 6:57 pm */

// TODO: CAW - Rename or remove a_iX
void RFont::DrawText(const char *a_pcText, TInt a_iX, TInt a_iY)
{

#ifdef __amigaos4__

#else /* ! __amigaos4__ */

	// TODO: CAW - Do this in open so it can be cached?
	TInt Height;
	TEXTMETRIC TextMetric;

	if (GetTextMetrics(m_poWindow->m_poDC, &TextMetric))
	{
		Height = TextMetric.tmHeight;

		// TODO: CAW - What about checking m_poDC is ok?
		//::DrawText(m_poWindow->m_poDC, a_pcText, strlen(a_pcText), &m_poWindow->m_oPaintStruct.rcPaint, 0);
		TextOut(m_poWindow->m_poDC, 0, (a_iY * Height), a_pcText, strlen(a_pcText));
	}

#endif /* ! __amigaos4__ */

}
