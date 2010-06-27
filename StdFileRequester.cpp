
#include "StdFuncs.h"
#include "StdFileRequester.h"

/* Written: Saturday 26-Jun-2010 2:48 pm */

TInt RFileRequester::GetFileName(TBool /*a_bOpen*/)
{
	TInt RetVal;
	OPENFILENAME OpenFileName;

	/* Initialise the OPENFILENAME structure to display the last filename we used, if any */

	memset(&OpenFileName, 0, sizeof(OpenFileName));
	OpenFileName.lStructSize = sizeof(OpenFileName);
	OpenFileName.lpstrFile = m_acFileName;
	OpenFileName.nMaxFile = MAX_FILEREQUESTER_PATH;

	/* Query the user for the filename to which to save */

	if (GetSaveFileName(&OpenFileName))
	{
		RetVal = KErrNone;
	}

	/* Determine if the dialog was closed by the user or if an error occurred */

	else
	{
		RetVal = (CommDlgExtendedError() == 0) ? KErrCancel : KErrGeneral;
	}

	return(RetVal);
}
