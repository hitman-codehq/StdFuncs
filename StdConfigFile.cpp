
#include "StdFuncs.h"
#include <string.h>
#include "File.h"
#include "Lex.h"
#include "StdConfigFile.h"

/* Written: Wednesday 22-Apr-1998 9:33 pm */
/* @param	a_pccFileName	Name of config file to open */
/* @return	KErrNone if successful */
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

		if ((m_pcBuffer = new char[m_iBufferSize]) != NULL)
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
/* @return	KErrNone if successful */
/*			KErrNoMemory if there was not enough memory to extract the key's value */
/*			KErrNotFound if a key with the specified name could not be found */
/*			KErrCorrupt if the key was found but its value was not a valid number */
/* Retrieves the value of an integer belonging to a user specified key, in a */
/* specified section and subsection.  Section, subsection and key names are case */
/* insensitive */

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

/* Written: Wednesday 22-Apr-1998 9:31 pm */
/* @param	a_pccSectionName	Name of section from which to read */
/*			a_pccSubSectionName	Name of subsection from which to read */
/*			a_pccKeyName		Name of key to be read */
/*			a_rpcResult			Reference to ptr into which to place ptr to retrieved string */
/* @return	KErrNone if successful */
/*			KErrNoMemory if there was not enough memory to extract the key's value */
/*			KErrNotFound if the key was not found due to the key being missing or */
/*						 the requested section and/or subsection not being found */
/* Retrieves the value of a string belonging to a user specified key, in a */
/* specified section and subsection.  Section, subsection and key names are case */
/* insensitive.  In the case where no key is found or an error occurs, the contents */
/* of a_rpcResult will be set to NULL */

TInt RConfigFile::GetString(const char *a_pccSectionName, const char *a_pccSubSectionName,
	const char *a_pccKeyName, char *&a_rpcResult)
{
	char *Buffer;
	const char *Token;
	TBool FoundSection, FoundSubSection;
	TInt BufferSize, Index, KeyNameLength, LFIndex, SectionNameLength, SubSectionNameLength, RetVal, TokenLength;

	/* Assume failure */

	a_rpcResult = NULL;
	RetVal = KErrNotFound;

	/* Setup some variables that point to the current position in the buffer and the */
	/* state of the search */

	FoundSection = FoundSubSection = EFalse;
	Buffer = m_pcBuffer;
	BufferSize = m_iBufferSize;
	KeyNameLength = strlen(a_pccKeyName);
	SectionNameLength = strlen(a_pccSectionName);
	SubSectionNameLength = strlen(a_pccSubSectionName);

	/* Iterate through the buffer containing the file and extract the key = value pairs */
	/* until such time as we have found the desired key */

	while (BufferSize > 0)
	{
		/* Find the index of the LF indicating the EOL as we only want to extract tokens */
		/* up until that point.  If this is a DOS style file then the TLex class will */
		/* automatically filter out the CR in the string to be parsed */

		LFIndex = 0;

		for (Index = 0; Index < BufferSize; ++Index)
		{
			if (Buffer[Index] == '\n')
			{
				LFIndex = Index;

				break;
			}
		}

		/* Initialise a TLex to parse the current line for tokens and configure it to treat */
		/* spaces, tabs and the = sign as white space.  This allows the user flexibility in */
		/* how they layout the .ini file */

		TLex Lex(Buffer, LFIndex);

		Lex.SetWhitespace("\t =");

		/* Update the buffer variables so they point to the start of the next line, for the */
		/* next iteration of the loop */

		++Index;
		Buffer += Index;
		BufferSize -= Index;

		/* Get the first token from the line */

		if ((Token = Lex.NextToken(&TokenLength)) != NULL)
		{
			/* If it is a comment then simply skip this entire line */

			if (Token[0] != ';')
			{
				/* If this is a section marker then see if it is the one we are interested */
				/* in.  If a section or subsection has already been found then abort processing */
				/* as duplicate sections of the same name are not supported and we have not */
				/* found the key we are looking for in the currently active section/subsection */

				if ((Token[0] == '(') && (Token[TokenLength - 1] == ')'))
				{
					if (!(FoundSubSection))
					{
						if (!(FoundSection))
						{
							/* Is this the section we are after?  Only do a comparison if the token */
							/* found is the same length as the section name (taking into account the */
							/* '(' and ')' characters surrounding the token) */

							if (SectionNameLength == (TokenLength - 2))
							{
								FoundSection = (strnicmp(&Token[1], a_pccSectionName, SectionNameLength) == 0);
							}
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}

				/* If we have found the target section and this is a subsection marker then see if */
				/* it is the one we are interested in.  Again, we abort processing if a subsection has */
				/* already been found */

				else if (FoundSection)
				{
					if ((Token[0] == '[') && (Token[TokenLength - 1] == ']'))
					{
						if (!(FoundSubSection))
						{
							/* Is this the subsection we are after?  Only do a comparison if the token */
							/* found is the same length as the subsection name (taking into account the */
							/* '[' and ']' characters surrounding the token) */

							if (SubSectionNameLength == (TokenLength - 2))
							{
								FoundSubSection = (strnicmp(&Token[1], a_pccSubSectionName, SubSectionNameLength) == 0);
							}
						}
						else
						{
							break;
						}
					}

					/* If we have found both the target section and the target subsection then see if */
					/* this is the key we are interested in */

					else if (FoundSubSection)
					{
						/* Is this the key we are after?  Only do a comparison if the token found is */
						/* the same length as the target key, to avoid finding keys that are substrings */
						/* of the token.  ie.  We don't want "Short" to match "ShortKey" */

						if ((TokenLength == KeyNameLength) && (strnicmp(Token, a_pccKeyName, KeyNameLength) == 0))
						{
							/* Yep!  Ket the key's value, skipping over any whitespace and = signs */
							/* between the key and its value */

							if ((Token = Lex.NextToken(&TokenLength)) != NULL)
							{
								/* Allocate a buffer for the token */

								if ((a_rpcResult = new char[TokenLength + 1]) != NULL)
								{
									RetVal = KErrNone;

									/* And extract the token into it */

									memcpy(a_rpcResult, Token, TokenLength);
									a_rpcResult[TokenLength] = '\0';

									break;
								}
								else
								{
									Utils::Info("RConfigFile::GetString() => Not enough memory to allocate string");

									RetVal = KErrNoMemory;
								}
							}
						}
					}
				}
			}
		}
	}

	return(RetVal);
}
