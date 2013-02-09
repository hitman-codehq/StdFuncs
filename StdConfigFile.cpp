
#include "StdFuncs.h"
#include <string.h>
#include "File.h"
#include "Lex.h"
#include "StdList.h"
#include "StdConfigFile.h"

/* Written: Saturday 09-Feb-2013 12:19 pm, Code HQ Ehinger Tor */
/* @param	a_pccKey		Ptr to the name of the key to be created */
/*			a_iKeyLength	Length of the string in a_pccKey */
/*			a_pccValue		Ptr to the value of the key to be created */
/*			a_iValueLength	Length of the string in a_pccValue */
/* @return	Ptr to the newly created key, if successful, else NULL */
/* Creates an instance of the CKey class and copies the specified key and value */
/* into it.  Memory will be allocated for these strings - they do not need to be */
/* in persistent storage */

CKey *CKey::New(const char *a_pccKey, TInt a_iKeyLength, const char *a_pccValue, TInt a_iValueLength)
{
	CKey *RetVal;

	/* Attempt to allocate an instance of the class and memory for the strings it will contain */

	if ((RetVal = new CKey) != NULL)
	{
		RetVal->m_pcName = new char[a_iKeyLength + 1];
		RetVal->m_pcValue = new char[a_iValueLength + 1];

		/* If both strings have been allocated successfully then copy their contents */
		/* into the newly allocated memory */

		if ((RetVal->m_pcName) && (RetVal->m_pcValue))
		{
			memcpy(RetVal->m_pcName, a_pccKey, a_iKeyLength);
			RetVal->m_pcName[a_iKeyLength] = '\0';

			memcpy(RetVal->m_pcValue, a_pccValue, a_iValueLength);
			RetVal->m_pcValue[a_iValueLength] = '\0';
		}

		/* Otherwise free up whatever has been allocated and return failure */

		else
		{
			delete RetVal;
			RetVal = NULL;
		}
	}

	return(RetVal);
}

/* Written: Saturday 09-Feb-2013 11:21 am, Code HQ Ehinger Tor */
/* @param	a_pccName		Ptr to the name of the section to be created */
/*			a_iLength		Length of the string in a_pccName */
/* @return	Ptr to the newly created section, if successful, else NULL */
/* Creates an instance of the CSection class and copies the specified name into */
/* it.  Memory will be allocated for this string - it does not need to be in */
/* persistent storage */

CSection *CSection::New(const char *a_pccName, TInt a_iLength)
{
	CSection *RetVal;

	/* Attempt to allocate an instance of the class and memory for the string it will contain */

	if ((RetVal = new CSection) != NULL)
	{
		if ((RetVal->m_pcName = new char[a_iLength + 1]) != NULL)
		{
			/* Copy the contents of the name into the newly allocated memory */

			memcpy(RetVal->m_pcName, a_pccName, a_iLength);
			RetVal->m_pcName[a_iLength] = '\0';
		}

		/* On failure, free up whatever has been allocated and return failure */

		else
		{
			delete RetVal;
			RetVal = NULL;
		}
	}

	return(RetVal);
}

/* Written: Saturday 09-Feb-2013 12:18 pm, Code HQ Ehinger Tor */
/* Frees up any resources associated with the section, including the list of embedded */
/* subsections and keys */

CSection::~CSection()
{
	CKey *Key;
	CSection *Section;

	delete [] m_pcName;

	/* Delete all of the subsections belonging to this section */

	while ((Section = m_oSections.RemHead()) != NULL)
	{
		delete Section;
	}

	/* Delete all of the keys and their values belonging to this section */

	while ((Key = m_oKeys.RemHead()) != NULL)
	{
		delete Key;
	}
}

/* Written: Saturday 09-Feb-2013 2:05 pm, Code HQ Ehinger Tor */
/* @return	The first subsection belonging to this section, or NULL */
/*			if there are no sections in the list */
/* Finds the first subsection instance within this section.  No kind of */
/* matching is performed - whatever is first in the subsection list is what */
/* is returned */

CSection *CSection::FindSection()
{
	return(m_oSections.GetHead());
}

/* Written: Saturday 09-Feb-2013 1:10 pm, Code HQ Ehinger Tor */
/* @param	a_pccName	Ptr to the name of the section for which to search */
/* @return	The first subsection that matches the name passed in, or NULL */
/*			if there are no matching sections in the list */
/* Finds the first subsection instance within this section that matches the */
/* name passed in */

CSection *CSection::FindSection(const char *a_pccName)
{
	CSection *RetVal;

	/* Get a ptr to the first subsection in the section */

	if ((RetVal = m_oSections.GetHead()) != NULL)
	{
		/* Loop around until a subsection is found that matches the name passed in */

		do
		{
			if (stricmp(RetVal->m_pcName, a_pccName) == 0)
			{
				break;
			}

			/* Not a matching subsection - get a ptr to the next subsection in the list */

			RetVal = m_oSections.GetSucc(RetVal);
		}
		while (RetVal);
	}

	return(RetVal);
}

/* Written: Saturday 09-Feb-2013 2:12 pm, Code HQ Ehinger Tor */
/* @param	a_poSection	Ptr to the section from which to search */
/* @return	The next subsection after the one specified, or NULL */
/*			if there are no more sections in the list */
/* Finds the next subsection instance after the one passed in.  No kind of */
/* matching is performed - whatever is next in the subsection list is what */
/* is returned */

CSection *CSection::FindNextSection(CSection *a_poSection)
{
	return(m_oSections.GetSucc(a_poSection));
}

/* Written: Saturday 09-Feb-2013 1:36 pm, Code HQ Ehinger Tor */
/* @param	a_pccName	Ptr to the name of the key for which to search */
/* @return	The first key that matches the name passed in, or NULL if */
/*			there are no matching sections in the list */
/* Finds the first key within this section that matches the name passed in */

CKey *CSection::FindKey(const char *a_pccName)
{
	CKey *RetVal;

	/* Get a ptr to the first key in the section */

	if ((RetVal = m_oKeys.GetHead()) != NULL)
	{
		/* Loop around until a key is found that matches the name passed in */

		do
		{
			if (stricmp(RetVal->m_pcName, a_pccName) == 0)
			{
				break;
			}

			/* Not a matching key - get a ptr to the next subsection in the list */

			RetVal = m_oKeys.GetSucc(RetVal);
		}
		while (RetVal);
	}

	return(RetVal);
}

/* Written: Saturday 09-Feb-2013 1:54 pm, Code HQ Ehinger Tor */
/* @param	a_poKey		Ptr to the key from which to search */
/*			a_pccName	Ptr to the name of the key for which to search */
/* @return	The next key that matches the name passed in, or NULL if */
/*			there are no matching sections in the list */
/* Finds the next key within this section that matches the name passed in. */
/* The search begins with the next key after the one passed in.  By using */
/* CSection::FindKey() in conjunction with this function it is possible to */
/* iterate through all of the keys of a given name that are present */

CKey *CSection::FindNextKey(CKey *a_poKey, const char *a_pccName)
{
	CKey *RetVal;

	/* Start searching at the key passed in */

	RetVal = a_poKey;

	/* Get a ptr to the next key in the list and loop around checking to see if it */
	/* matches the one passed in */

	while ((RetVal = m_oKeys.GetSucc(RetVal)) != NULL)
	{
		if (stricmp(RetVal->m_pcName, a_pccName) == 0)
		{
			break;
		}
	}

	return(RetVal);
}

/* Written: Wednesday 22-Apr-1998 9:33 pm */
/* @param	a_pccFileName	Name of configuration file to open */
/* @return	KErrNone if successful */
/*			KErrNoMemory if there was not enough memory to read the file */
/*			KErrEof if the file could be opened but not completely read */
/*			Any error from Utils::GetFileInfo() */
/*			Any error from RFile::Open() */
/* Opens a configuration file and reads its contents into memory in preparation for parsing */

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

			/* Read the file in in its entirity */

			if ((RetVal = File.Open(a_pccFileName, EFileRead)) == KErrNone)
			{
				if (File.Read((unsigned char *) m_pcBuffer, m_iBufferSize) == m_iBufferSize)
				{
					/* And parse it into lists of sections, subsections, groups and keys */

					RetVal = Parse();
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

	/* If anything failed then clean up */

	if (RetVal != KErrNone)
	{
		Close();
	}

	return(RetVal);
}

/* Written: Wednesday 22-Apr-1998 9:35 pm */
/* Frees any resources associated with the configuration file */

void RConfigFile::Close()
{
	CSection *Section;

	delete [] m_pcBuffer;
	m_pcBuffer = NULL;

	/* Delete all of the sections found in the configuration file */

	while ((Section = m_oSections.RemHead()) != NULL)
	{
		delete Section;
	}
}

/* Written: Saturday 09-Feb-2013 1:04 pm, Code HQ Ehinger Tor */
/* @param	a_pccName	Ptr to the name of the section for which to search */
/* @return	The first section that matches the name passed in, or NULL */
/*			if there are no matching sections in the configuration file */
/* Finds the first top level section instance within this configuration file that matches */
/* the name passed in */

CSection *RConfigFile::FindSection(const char *a_pccName)
{
	CSection *RetVal;

	if ((RetVal = m_oSections.GetHead()) != NULL)
	{
		do
		{
			if (stricmp(RetVal->m_pcName, a_pccName) == 0)
			{
				break;
			}

			RetVal = m_oSections.GetSucc(RetVal);
		}
		while (RetVal);
	}

	return(RetVal);
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

/* Written: Saturday 09-Feb-2013 11:00 am, Code HQ Ehinger Tor */

TInt RConfigFile::Parse()
{
	char *Buffer, Message[256]; // TODO: CAW - Temporary
	const char *Token, *ValueToken;
	TInt BufferSize, Index, LFIndex, RetVal, TokenLength, ValueTokenLength;
	CKey *Key;
	CSection *NewSection, *Section, *SubSection, *Group;

	/* Assume failure */

	RetVal = KErrNoMemory;

	/* Setup some variables that point to the current position in the buffer and the */
	/* state of the search */

	Buffer = m_pcBuffer;
	BufferSize = m_iBufferSize;
	Section = SubSection = Group = NULL;

	/* Iterate through the buffer containing the file and extract all section, subsections, */
	/* groups and key = value pairs */

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
				/* If this is a valid section marker then allocate a CSection instance for it */
				/* and add it to the list of top level sections */

				if ((Token[0] == '(') && (Token[TokenLength - 1] == ')'))
				{
					if ((NewSection = CSection::New(&Token[1], (TokenLength - 2))) != NULL)
					{
						/* Make this the current section and indicate that there are no longer */
						/* any currently active subsections or groups.  Otherwise any newly found */
						/* groups or key would be added to the incorrect subsection */

						Section = NewSection;
						SubSection = Group = NULL;

						/* And add the section to the list of top level sections */

						m_oSections.AddTail(NewSection);
					}
					else
					{
						break;
					}
				}

				/* If this is a valid subsection marker then allocate a CSection instance for it */
				/* and add it to the list of subsections in the current top level section */

				else if ((Token[0] == '[') && (Token[TokenLength - 1] == ']'))
				{
					/* Only use the subsection if it is within a section */

					if (Section)
					{
						if ((NewSection = CSection::New(&Token[1], (TokenLength - 2))) != NULL)
						{
							/* Indicate that this is now the currently active subsection and that */
							/* there is no longer an active group */

							SubSection = NewSection;
							Group = NULL;

							/* And add the subsection to the current section's list of subsections */

							Section->m_oSections.AddTail(NewSection);
						}
						else
						{
							break;
						}
					}

					/* Otherwise the subsection is invalid so discard it and display a warning */

					else
					{
						memcpy(Message, &Token[1], (TokenLength - 2));
						Message[TokenLength - 2] = '\0';
						Utils::Info("Discarding orphan subsection %s", Message);
					}
				}

				/* If this is a valid group marker then allocate a CSection instance for it */
				/* and add it to the list of subsections in the current subsection */

				else if ((Token[0] == '{') && (Token[TokenLength - 1] == '}'))
				{
					/* Only use the subsection if it is within a subsection */

					if (SubSection)
					{
						if ((NewSection = CSection::New(&Token[1], (TokenLength - 2))) != NULL)
						{
							/* Indicate that this is now the currently active group */

							Group = NewSection;

							/* And add the group to the current subsection's list of groups */

							SubSection->m_oSections.AddTail(NewSection);
						}
						else
						{
							break;
						}
					}

					/* Otherwise the group is invalid so discard it and display a warning */

					else
					{
						memcpy(Message, &Token[1], (TokenLength - 2));
						Message[TokenLength - 2] = '\0';
						Utils::Info("Discarding orphan group %s", Message);
					}
				}

				/* If this is a valid key and value pair CKey instance for it */

				else if ((ValueToken = Lex.NextToken(&ValueTokenLength)) != NULL)
				{
					/* Only use the key and value if it is within a subsection or a group */

					if ((SubSection) || (Group))
					{
						if ((Key = CKey::New(Token, TokenLength, ValueToken, ValueTokenLength)) != NULL)
						{
							/* Add the new key to the current group, if there is one.  Otherwise add */
							/* it to the current subsection */

							if (Group)
							{
								Group->m_oKeys.AddTail(Key);
							}
							else
							{
								SubSection->m_oKeys.AddTail(Key);
							}
						}
						else
						{
							break;
						}
					}

					/* Otherwise the key is invalid so discard it and display a warning */

					else
					{
						memcpy(Message, Token, TokenLength);
						Message[TokenLength] = '\0';
						Utils::Info("Discarding orphan key %s", Message);
					}
				}
			}
		}
	}

	/* If we reached the end of the buffer without breaking out of the loop then the parsing */
	/* was successful.  If the loop was broken out of then it means an error occurred.  If the */
	/* buffer was empty (due to the file being 0 bytes long) then this condition will still */
	/* trigger and it will be considered a successful parse, although with no sections found */

	if (BufferSize <= 0)
	{
		RetVal = KErrNone;
	}

	return(RetVal);
}
