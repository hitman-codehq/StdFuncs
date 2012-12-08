
#include "StdFuncs.h"
#include <string.h>
#include "File.h"
#include "Lex.h"
#include "StdConfigFile.h"

/***********************************************************/
/* ConfigFile::OpenConfigFile will open a config file for  */
/* reading.                                                */
/* Written: Wednesday 22-Apr-1998 9:33 pm                  */
/* Passed: pcConfigFileName => Name of config file to open */
/* Returns: CFE_Ok if successful, else CFE_CouldntOpenFile */
/***********************************************************/

TInt RConfigFile::Open(const char *a_pcFileName)
{
	TInt RetVal;
	TEntry Entry;

	if ((RetVal = Utils::GetFileInfo(a_pcFileName, &Entry)) == KErrNone)
	{
		m_iBufferSize = Entry.iSize;

		if ((m_pcBuffer = new char[m_iBufferSize]) != NULL) // TODO: CAW - +1?  What about NULL termination?
		{
			RFile File;

			if ((RetVal = File.Open(a_pcFileName, EFileRead)) == KErrNone)
			{
				if (File.Read((unsigned char *) m_pcBuffer, m_iBufferSize) == m_iBufferSize)
				{
					RetVal = KErrNone;
				}
				else
				{
					RetVal = KErrEof;
				}

				File.Close();
			}
		}
		else
		{
			RetVal = KErrNoMemory;
		}
	}

	return(RetVal);
}

/**********************************************/
/* ConfigFile::CloseConfigFile will close the */
/* config file, if it is open                 */
/* Written: Wednesday 22-Apr-1998 9:35 pm     */
/**********************************************/

void RConfigFile::Close()
{
	delete [] m_pcBuffer;
	m_pcBuffer = NULL;
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

// TODO: CAW - Check validity of returned integer (SetLastError(CFE_BadNumber)) and maximum length of sConfigString?
void RConfigFile::GetInteger(const char *a_pccSectionName, const char *a_pccSubSectionName,
	const char *a_pccKeyName, int *a_piResult)
{
	char ConfigString[MAX_PATH];

	GetString(a_pccSectionName, a_pccSubSectionName, a_pccKeyName, ConfigString);
	*a_piResult= atoi(ConfigString);
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

// TODO: CAW - Error handling: CFE_SectionNotFound, CFE_SubSectionNotFound, CFE_KeyNotFound
void RConfigFile::GetString(const char *a_pccSectionName, const char *a_pccSubSectionName,
	const char *a_pccKeyName, char *a_pcResult)
{
	char *Buffer;
	const char *Token;
	TBool Done, FoundSection, FoundSubSection;
	TInt BufferSize, Index, CRIndex, LFIndex, TokenLength;

	*a_pcResult = '\0';

	Done = FoundSection = FoundSubSection = EFalse;

	Buffer = m_pcBuffer;
	BufferSize = m_iBufferSize;

	while (BufferSize > 0)
	{
		CRIndex = LFIndex = 0;

		for (Index = 0; Index < BufferSize; ++Index)
		{
			if (Buffer[Index] == '\r')
			{
				CRIndex = Index;
			}

			if (Buffer[Index] == '\n')
			{
				LFIndex = Index;

				break;
			}
		}

		TLex Lex(Buffer, LFIndex);

		++Index;
		Buffer += Index;
		BufferSize -= Index;

		if ((Token = Lex.NextToken(&TokenLength)) != NULL)
		{
			if (Token[0] != ';')
			{
				if (Token[0] == '(')
				{
					if (!(FoundSubSection))
					{
						if (!(FoundSection))
						{
							// TODO: CAW - This could run over the end of Token
							FoundSection = (!(strnicmp(&Token[1], a_pccSectionName, strlen(a_pccSectionName))));
							FoundSubSection = EFalse;
						}
						else
						{
							//Done = ETrue;
						}
					}
					else
					{
						//Done = ETrue;
					}
				}
				else
				{
					if (FoundSection)
					{
						if (Token[0] == '[')
						{
							if (!(FoundSubSection))
							{
								// TODO: CAW - This could run over the end of Token
								FoundSubSection = (!(strnicmp(&Token[1], a_pccSubSectionName, strlen(a_pccSubSectionName))));
							}
							else
							{
								//Done = ETrue;
							}
						}
						else
						{
							if (FoundSubSection)
							{
								if (!(strnicmp(Token, a_pccKeyName, strlen(a_pccKeyName)))) // TODO: CAW - Slow, here and elsewhere
								{
									Token = Lex.NextToken(&TokenLength);

									if ((Token) && (strncmp(Token, "=", 1) == 0))
									{
										Token = Lex.NextToken(&TokenLength);

										if (Token)
										{
											/* Extract the token into the dest buffer and signal success */

											memcpy(a_pcResult, Token, TokenLength);
											a_pcResult[TokenLength] = '\0';

											break;
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
}
