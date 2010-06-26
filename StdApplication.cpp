
#include "StdFuncs.h"
#include "StdApplication.h"
#include "StdWindow.h"

/* Written: Saturday 26-Jun-2010 11:53 am */

RApplication::RApplication()
{

#ifdef WIN32

	m_poAccelerators = NULL;

#endif /* WIN32 */

}

/* Written: Monday 08-Feb-2010 6:54 am */

int RApplication::Main()
{

#ifdef __amigaos4__

	bool Done;
	int Code;
	ULONG Signal, WindowSignal;
	struct IntuiMessage *IntuiMessage;

	ASSERTM(m_poWindow, "RApplication::Main() => Window handle must not be NULL");

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

					case IDCMP_RAWKEY :
					{
						Code = IntuiMessage->Code;

						if (((Code >= STD_KEY_PGUP) && (Code <= STD_KEY_LEFT)) || ((Code >= STD_KEY_HOME) && (Code <= STD_KEY_END)))
						{
							m_poWindow->OfferKeyEvent(IntuiMessage->Code);
						}

						break;
					}

					case IDCMP_VANILLAKEY :
					{
						m_poWindow->OfferKeyEvent(IntuiMessage->Code);

						break;
					}

					case IDCMP_REFRESHWINDOW :
					{
						m_poWindow->Draw();

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

	/* Try to load the default accelerator table.  If this is not found then this is not an error; */
	/* just continue without accelerators */

	m_poAccelerators = LoadAccelerators(GetModuleHandle(NULL), MAKEINTRESOURCE(103));

	/* Standard Windows message loop with accelerator handling */

	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		/* Try to translate the message;  Windows will handle m_poAccelerators being NULL */

		if (TranslateAccelerator(m_poWindow->m_poWindow, m_poAccelerators, &Msg) == 0)
		{
			/* No accelerator found so do the standard message translation and despatch */

			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
	}

	/* Destroy the accelerators if they have been loaded */

	if (m_poAccelerators)
	{
		DestroyAcceleratorTable(m_poAccelerators);
	}

#endif /* ! __amigaos4__ */

	return(KErrNone);
}

/* Written: Monday 08-Feb-2010 7:25 am */

void RApplication::AddWindow(CWindow *a_poWindow)
{
	m_poWindow = a_poWindow;
}
