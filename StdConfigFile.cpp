
#include "StdFuncs.h"
#include "Lex.h"
#include "StdConfigFile.h"

/***********************************************************/
/* ConfigFile::OpenConfigFile will open a config file for  */
/* reading.                                                */
/* Written: Wednesday 22-Apr-1998 9:33 pm                  */
/* Passed: pcConfigFileName => Name of config file to open */
/* Returns: CFE_Ok if successful, else CFE_CouldntOpenFile */
/***********************************************************/

TInt RConfigFile::OpenConfigFile(const char *pcConfigFileName)
{
	TInt RetVal;
	TEntry Entry;

	if (Utils::GetFileInfo(pcConfigFileName, &Entry) == KErrNone)
	{
		iBufferSize = Entry.iSize;

		if ((pBuffer = new char[iBufferSize]) != NULL) // TODO: CAW - +1?  What about NULL termination?
		{
			if ((RetVal = fConfigFile.Open(pcConfigFileName, EFileRead)) == KErrNone)
			{
				if (fConfigFile.Read((unsigned char *) pBuffer, iBufferSize) == iBufferSize) // TODO: CAW - Cast
				{
					bConfigFileOpen = TRUE;
				} // TODO: CAW - Else
			}
		}
	}

	return(RetVal);
}

/**********************************************/
/* ConfigFile::CloseConfigFile will close the */
/* config file, if it is open                 */
/* Written: Wednesday 22-Apr-1998 9:35 pm     */
/**********************************************/

void RConfigFile::CloseConfigFile()
{
  if (bConfigFileOpen)
  {
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
void RConfigFile::GetConfigInteger(const char *pcSectionName, const char *pcSubSectionName,
	const char *pcKeyName, int *piDestInt)
{
  //String
  char sConfigString[MAX_PATH];

  GetConfigString(pcSectionName, pcSubSectionName, pcKeyName, sConfigString);
  *piDestInt = atoi(sConfigString);

  // TODO: CAW
  //if (!(Platform::atoi(sConfigString, piDestInt)))
  {
    //SetLastError(CFE_BadNumber);
    //THROW1(ConfigFileException, GetLastErrorString());
  }
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

void RConfigFile::GetConfigString(const char *pcSectionName, const char *pcSubSectionName,
	const char *pcKeyName, char *pcDestString)
{
  char *Buffer;
  const char *Token;
  TInt BufferSize, Index, TokenLength;
  BOOL bDone, bFoundSection, bFoundSubSection;

  //SetLastError(CFE_KeyNotFound);
  bDone = bFoundSection = bFoundSubSection = FALSE;

  Buffer = pBuffer;
  BufferSize = iBufferSize;

  //try // TODO: CAW - Check how this affects the size
  {
    while ((!(bDone)))// && /*(ceLastError == CFE_KeyNotFound) &&*/ (!(fConfigFile.Eof())))
    {
      for (Index = 0; Index < BufferSize; ++Index)
	  {
	    // TODO: CAW - Test for this causing problems with TLex
		if (Buffer[Index] == '\r')
		{
		  Buffer[Index] = '\0';
		}

		if (Buffer[Index] == '\n')
		{
		  Buffer[Index] = '\0';

          break;
		}
	  }

	  TLex Lex(Buffer, Index);

	  BufferSize -= Index;
	  ++Index;
	  Buffer += Index;

	  if ((Token = Lex.NextToken(&TokenLength)) != NULL)
      {
        if (Token[0] != ';')
        {
          if (Token[0] == '(')
          {
            if (!(bFoundSubSection))
            {
              if (!(bFoundSection))
              {
			    // TODO: CAW - This could run over the end of Token
                bFoundSection = (!(strnicmp(&Token[1], pcSectionName, strlen(pcSectionName))));
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
              if (Token[0] == '[')
              {
                if (!(bFoundSubSection))
                {
			      // TODO: CAW - This could run over the end of Token
                  bFoundSubSection = (!(strnicmp(&Token[1], pcSubSectionName, strlen(pcSubSectionName))));
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
                  if (!(strnicmp(Token, pcKeyName, strlen(pcKeyName)))) // TODO: CAW - Slow, here and elsewhere
                  {
					Token = Lex.NextToken(&TokenLength);

					if ((Token) && (strncmp(Token, "=", 1) == 0))
					{
					Token = Lex.NextToken(&TokenLength);

					if (Token)
					{
                    /* Copy the string into the dest buffer and signal success */

                    memcpy(pcDestString, Token, TokenLength);
  					pcDestString[TokenLength] = '\0'; // TODO: CAW - Can strncpy be used?
                    bDone = TRUE;
					}
					}
                  }
                }
              }
            }
          }
        }
      }
    }

    /*if (ceLastError != CFE_Ok)
    {
      if (!(bFoundSection))
      {
        SetLastError(CFE_SectionNotFound);
      }
      else if (!(bFoundSubSection))
      {
        SetLastError(CFE_SubSectionNotFound);
      }
    }*/
  }
  //catch(FileEx &)
  {
    //SetLastError(CFE_FileIOError);
  }

  //if (ceLastError != CFE_Ok)
  {
    //THROW1(ConfigFileException, GetLastErrorString());
  }
}
