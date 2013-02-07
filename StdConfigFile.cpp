
#include "StdFuncs.h"
#include <string.h>
#include "File.h"
#include "Lex.h"
#include "StdConfigFile.h"

/* Written: Wednesday 22-Apr-1998 9:33 pm */
/* @param	a_pccFileName	Name of config file to open */
/* @returns	KErrNone if successful */
/*			KErrNoMemory if there was not enough memory to read the file */
/*			KErrEof if the file could be opened but not completely read */
/*			Any error from Utils::GetFileInfo() */
/*			Any error from RFile::Open() */
/* Opens a config file and reads its contents into memory in preparation for parsing */

TInt RConfigFile::Open(const char *a_pccFileName)
{
	TInt RetVal;
	TEntry Entry;

	/* Examine the file to determine its size and allocate a buffer large */
	/* enough to hold it */

	if ((RetVal = Utils::GetFileInfo(a_pccFileName, &Entry)) == KErrNone)
	{
		m_iBufferSize = Entry.iSize;

		if ((m_pcBuffer = new char[m_iBufferSize]) != NULL) // TODO: CAW - +1?  What about NULL termination?
		{
			RFile File;

			/* And read the file in in its entirity */

			if ((RetVal = File.Open(a_pccFileName, EFileRead)) == KErrNone)
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

/* Written: Wednesday 22-Apr-1998 9:35 pm */
/* Frees any resources associated with the config file */

void RConfigFile::Close()
{
	delete [] m_pcBuffer;
	m_pcBuffer = NULL;
}

/* Written: Wednesday 29-Apr-1998 11:08 pm */
/* @param	a_pccpcSectionName	Ptr to name of section the key is in */
/*			a_pccSubSectionName	Ptr to name of subsection the key is in in */
/*			a_pccKeyName		Ptr to name of key to read */
/*			a_piResult			Ptr to integer variable into which to place */
/*								the retrieved key value */
/* @returns	KErrNone if successful */
/*			KErrNotFound if a key with the specified name could not be found */
/*			KErrCorrupt if the key was found but its value was not a valid number */
/* Retrieves the value of an integer belonging to a user specified key, in a */
/* specified section and subsection */

TInt RConfigFile::GetInteger(const char *a_pccSectionName, const char *a_pccSubSectionName,
	const char *a_pccKeyName, TInt *a_piResult)
{
	char Char, *ConfigString;
	TInt Index, RetVal;

	/* Get the string associated with the requested setting */

	GetString(a_pccSectionName, a_pccSubSectionName, a_pccKeyName, ConfigString);

	/* If the string was obtained successfully, convert it to an integer and */
	/* and save it, and free the string */

	if (ConfigString)
	{
		/* First, manually confirm that the string returned is a valid number, as atoi() does */
		/* not differentiate between '0' and an invalid number - both return 0! */

		Index = 0;

		while ((Char = ConfigString[Index]) != '\0')
		{
			if ((Char < '0') || (Char > '9'))
			{
				break;
			}

			++Index;
		}

		/* If we made it to the end of the string then we have a valid number */

		if (Char == '\0')
		{
			RetVal = KErrNone;

			/* Convert the valid number to an integer and return it */

			*a_piResult= atoi(ConfigString);
		}
		else
		{
			RetVal = KErrCorrupt;
		}

		delete [] ConfigString;
	}
	else
	{
		RetVal = KErrNotFound;
	}

	return(RetVal);
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

// TODO: CAW - Error handling: CFE_SectionNotFound, CFE_SubSectionNotFound, CFE_KeyNotFound, CFE_NoMemory
void RConfigFile::GetString(const char *a_pccSectionName, const char *a_pccSubSectionName,
	const char *a_pccKeyName, char *&a_rpcResult)
{
	char *Buffer;
	const char *Token;
	TBool FoundSection, FoundSubSection;
	TInt BufferSize, Index, LFIndex, TokenLength;

	/* Assume failure */

	a_rpcResult = NULL;

	FoundSection = FoundSubSection = EFalse;

	Buffer = m_pcBuffer;
	BufferSize = m_iBufferSize;

	while (BufferSize > 0)
	{
		LFIndex = 0;

		for (Index = 0; Index < BufferSize; ++Index)
		{
			if (Buffer[Index] == '\n')
			{
				LFIndex = Index;

				break;
			}
		}

		TLex Lex(Buffer, LFIndex);

		Lex.SetWhitespace("\t =");

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
									//Token = Lex.NextToken(&TokenLength);

									//if ((Token) && (strncmp(Token, "=", 1) == 0))
									{
										Token = Lex.NextToken(&TokenLength);

										if (Token)
										{
											/* Allocate a buffer for the token */

											a_rpcResult = new char[TokenLength + 1];

											if (a_rpcResult)
											{
												/* And extract the token into */

												memcpy(a_rpcResult, Token, TokenLength);
												a_rpcResult[TokenLength] = '\0';

												break;
											}
											else
											{
												Utils::Info("RConfigFile::GetString() => Not enough memory to allocate string");
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
}
