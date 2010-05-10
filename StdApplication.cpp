
#include "StdFuncs.h"
#include "StdApplication.h"
#include "StdWindow.h"

/* Written: Monday 08-Feb-2010 6:54 am */

int RApplication::Main()
{

#ifdef __amigaos4__

	bool Done;
	ULONG Signal, WindowSignal;
	struct IntuiMessage *IntuiMessage;

	Done = false;
	WindowSignal = m_poWindow->GetSignal();

	do
	{
		Signal = IExec->Wait(WindowSignal);

		if (Signal & WindowSignal)
		{
			while ((IntuiMessage = (struct IntuiMessage *) IExec->GetMsg(m_poWindow->GetWindow()->UserPort)) != NULL)
			{
				switch (IntuiMessage->Class)
				{
					case IDCMP_CLOSEWINDOW :
					{
						Done = true;

						break;
					}
				}

				/* And reply to the processed message */

				IExec->ReplyMsg((struct Message *) IntuiMessage);
			}
		}
	}
	while (!(Done));

#else /* ! __amigaos4__ */

	MSG Msg;

	/* Standard Windows message loop */

	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

#endif /* ! __amigaos4__ */

	return(KErrNone);
}

/* Written: Monday 08-Feb-2010 7:25 am */

void RApplication::AddWindow(CWindow *a_poWindow)
{
	m_poWindow = a_poWindow;
}
