
#include "StdFuncs.h"
#include "StdFont.h"
#include "StdWindow.h"

/* Written: Sunday 09-May-2010 6:57 pm */

void RFont::DrawText(const char *a_pcText, TInt a_iLength, TInt a_iY)
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
		TextOut(m_poWindow->m_poDC, 0, (a_iY * Height), a_pcText, a_iLength);
	}

#endif /* ! __amigaos4__ */

}
