
#include "StdFuncs.h"
#include "StdConfigFile.h"

/**********************************************/
/* ConfigFile::CloseConfigFile will close the */
/* config file, if it is open                 */
/* Written: Wednesday 22-Apr-1998 9:35 pm     */
/**********************************************/

void RConfigFile::CloseConfigFile()
{
  if (bConfigFileOpen)
  {
	IDOS->Printf("*** Closing config file\n");
	fConfigFile.Close();
    bConfigFileOpen = FALSE;
  }
}

/*********************************************************************/
/* ConfigFile::GetConfigInteger will get the integer belonging to a  */
/* user specified key, in a specified section and subsection.        */
/* Written: Wednesday 29-Apr-1998 11:08 pm                           */
/* Passed: pcSectionName => Ptr to name of section to check in       */
/*         pcSubSectionName => Ptr to name of subsection to check in */
/*         pcKeyName => Ptr to name of key to read string for        */
/*         piDestInt => Ptr to integer variable into which to place  */
/*                         the retrieved integer                     */
/* Returns: See ConfigFile::GetConfigString() for return details     */
/*********************************************************************/

// TODO: CAW - What to do about SectionName?
void RConfigFile::GetConfigInteger(const char *pcSectionName, const char *pcSubSectionName, const char *pcKeyName, int *piDestInt)
{
  //String sConfigString;

  //GetConfigString(pcSectionName, pcSubSectionName, pcKeyName, sConfigString);

	// TODO: CAW
  /*if (!(Platform::atoi(sConfigString, piDestInt)))
  {
    SetLastError(CFE_BadNumber);
    THROW1(ConfigFileException, GetLastErrorString());
  }*/
}

/***************************************************************************/
/* ConfigFile::GetConfigString will get the string belonging to a user     */
/* specified key, in a specified section and subsection.                   */
/* Written: Wednesday 22-Apr-1998 9:31 pm                                  */
/* Passed: pcSectionName => Ptr to name of section to check in             */
/*         pcSubSectionName => Ptr to name of subsection to check in       */
/*         pcKeyName => Ptr to name of key to read string for              */
/*         rsDestString => Reference to String object into which to place  */
/*                         the retrieved string                            */
/* Returns: Nothing, although it will throw a ConfigFileException if there */
/*          is a problem reading the configuration string.  In this case,  */
/*          you can find the error that occurred bycalling                 */
/*          ConfigFile::GetLastError() and the string of the error that    */
/*          occurred by calling ConfigFile::GetLastErrorString()           */
/***************************************************************************/

void RConfigFile::GetConfigString(const char *pcSectionName, const char *pcSubSectionName, const char *pcKeyName, char *pcDestString)
{
  char acTempString[MAX_PATH];
  ULONG ulEndIndex, ulIndex, ulLength;
  BOOL bDone, bFoundSection, bFoundSubSection;

  //SetLastError(CFE_KeyNotFound);
  bDone = bFoundSection = bFoundSubSection = FALSE;

#if 0
  try
  {
    fConfigFile.SeekFile(0, FSEEK_SET);

    while ((!(bDone)) && (ceLastError == CFE_KeyNotFound) && (!(fConfigFile.Eof())))
    {
      if (fConfigFile.ReadLine(acTempString, sizeof(acTempString)))
      {
        ulIndex = 0;

        while ((acTempString[ulIndex]) && (acTempString[ulIndex] == ' '))
        {
          ++ulIndex;
        }

        if (acTempString[ulIndex] != ';')
        {
          if (acTempString[ulIndex] == '(')
          {
            if (!(bFoundSubSection))
            {
              if (!(bFoundSection))
              {
                bFoundSection = (!(strnicmp(&acTempString[ulIndex + 1], pcSectionName, lstrlen(pcSectionName))));
                bFoundSubSection = FALSE;
              }
              else
              {
                bDone = TRUE;
              }
            }
            else
            {
              bDone = TRUE;
            }
          }
          else
          {
            if (bFoundSection)
            {
              if (acTempString[ulIndex] == '[')
              {
                if (!(bFoundSubSection))
                {
                  bFoundSubSection = (!(strnicmp(&acTempString[ulIndex + 1], pcSubSectionName, lstrlen(pcSubSectionName))));
                }
                else
                {
                  bDone = TRUE;
                }
              }
              else
              {
                if (bFoundSubSection)
                {
                  ulLength = lstrlen(pcKeyName);

                  if (!(strnicmp(&acTempString[ulIndex], pcKeyName, ulLength)))
                  {
                    /* Skip past any white space and the '=' operator */

                    ulIndex += ulLength;

                    while ((acTempString[ulIndex]) && ((acTempString[ulIndex] == ' ') || (acTempString[ulIndex] == '=')))
                    {
                      ++ulIndex;
                    }

                    /* Scan to the end of the string and delete any CRLF found */

                    ulEndIndex = ulIndex;

                    while ((acTempString[ulEndIndex]) && (acTempString[ulEndIndex] != '\r') && (acTempString[ulEndIndex] != '\n'))
                    {
                      ++ulEndIndex;
                    }

                    acTempString[ulEndIndex] = '\0';

                    /* Copy the string into the dest buffer and signal success */

                    rsDestString = &acTempString[ulIndex];
                    SetLastError(CFE_Ok);
                    bDone = TRUE;
                  }
                }
              }
            }
          }
        }
      }
    }

    if (ceLastError != CFE_Ok)
    {
      if (!(bFoundSection))
      {
        SetLastError(CFE_SectionNotFound);
      }
      else if (!(bFoundSubSection))
      {
        SetLastError(CFE_SubSectionNotFound);
      }
    }
  }
  catch(FileEx &)
  {
    SetLastError(CFE_FileIOError);
  }

  if (ceLastError != CFE_Ok)
  {
    THROW1(ConfigFileException, GetLastErrorString());
  }
#endif
}

/***********************************************************/
/* ConfigFile::OpenConfigFile will open a config file for  */
/* reading.                                                */
/* Written: Wednesday 22-Apr-1998 9:33 pm                  */
/* Passed: pcConfigFileName => Name of config file to open */
/* Returns: CFE_Ok if successful, else CFE_CouldntOpenFile */
/***********************************************************/

// TODO: CAW - Ordering
TInt RConfigFile::OpenConfigFile(const char *pcConfigFileName)
{
	TInt RetVal;

	IDOS->Printf("*** Opening config file\n");
	if ((RetVal = fConfigFile.Open(pcConfigFileName, (EFileRead | EFileWrite))) == KErrNone)
	{
		IDOS->Printf("*** OK\n");

		bConfigFileOpen = TRUE;
	}

	return(RetVal);
}

