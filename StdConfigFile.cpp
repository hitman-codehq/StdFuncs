
#include "StdFuncs.h"
#include <string.h>
#include "File.h"
#include "Lex.h"
#include "StdConfigFile.h"

/**
 * Creates an instance of the CKey class.
 * Creates an instance of the CKey class and copies the specified key and value
 * into it.  Memory will be allocated for these strings - they do not need to be
 * in persistent storage.
 *
 * @date	Saturday 09-Feb-2013 12:19 pm, Code HQ Ehinger Tor
 * @param	a_pccKey		Ptr to the name of the key to be created
 * @param	a_iKeyLength	Length of the string in a_pccKey
 * @param	a_pccValue		Ptr to the value of the key to be created
 * @param	a_iValueLength	Length of the string in a_pccValue
 * @return	Ptr to the newly created key, if successful, else NULL
 */

CKey *CKey::New(const char *a_pccKey, TInt a_iKeyLength, const char *a_pccValue, TInt a_iValueLength)
{
	CKey *RetVal;

	/* Attempt to allocate an instance of the class and memory for the strings it will contain */

	if ((RetVal = new CKey) != NULL)
	{
		RetVal->m_pcName = Utils::DuplicateString(a_pccKey, a_iKeyLength);
		RetVal->m_pcValue = Utils::DuplicateString(a_pccValue, a_iValueLength);

		/* If either string was not allocated successfully then free up whatever has been */
		/* allocated and return failure */

		if ((!(RetVal->m_pcName)) || (!(RetVal->m_pcValue)))
		{
			delete RetVal;
			RetVal = NULL;
		}
	}

	return(RetVal);
}

/**
 * Creates an instance of the CSection class.
 * Creates an instance of the CSection class and copies the specified name into
 * it.  Memory will be allocated for this string - it does not need to be in
 * persistent storage.
 *
 * @date	Saturday 09-Feb-2013 11:21 am, Code HQ Ehinger Tor
 * @param	a_pccName		Ptr to the name of the section to be created
 * @param	a_iLength		Length of the string in a_pccName
 * @return	Ptr to the newly created section, if successful, else NULL
 */

CSection *CSection::New(const char *a_pccName, TInt a_iLength)
{
	CSection *RetVal;

	/* Attempt to allocate an instance of the class and memory for the string it will contain */

	if ((RetVal = new CSection) != NULL)
	{
		if ((RetVal->m_pcName = Utils::DuplicateString(a_pccName, a_iLength)) == NULL)
		{
			delete RetVal;
			RetVal = NULL;
		}
	}

	return(RetVal);
}

/**
 * Destructor for the CSection class.
 * Frees up any resources associated with the section, including the list of embedded
 * subsections and keys.
 *
 * @date	Saturday 09-Feb-2013 12:18 pm, Code HQ Ehinger Tor
 */

CSection::~CSection()
{
	CKey *Key;
	CSection *Section;

	delete [] m_pcName;

	/* Delete all of the subsections belonging to this section */

	while ((Section = m_oSections.remHead()) != NULL)
	{
		delete Section;
	}

	/* Delete all of the keys and their values belonging to this section */

	while ((Key = m_oKeys.remHead()) != NULL)
	{
		delete Key;
	}
}

/**
 * Finds the first subsection belonging to this section.
 * Finds the first subsection within this section.  No kind of matching
 * is performed - whatever is first in the subsection list is what is
 * returned.
 *
 * @date	Saturday 09-Feb-2013 2:05 pm, Code HQ Ehinger Tor
 * @return	The first subsection belonging to this section, or NULL
 *			if there are no sections in the list
 */

CSection *CSection::FindSection()
{
	return(m_oSections.GetHead());
}

/**
 * Finds the first subsection of a given name belonging to this section.
 * Finds the first subsection within this section that matches the name
 * passed in.
 *
 * @date	Saturday 09-Feb-2013 1:10 pm, Code HQ Ehinger Tor
 * @param	a_pccName	Ptr to the name of the section for which to search
 * @return	The first subsection that matches the name passed in, or NULL
 *			if there are no matching sections in the list
 */

CSection *CSection::FindSection(const char *a_pccName)
{
	CSection *RetVal;

	/* Get a ptr to the first subsection in the section */

	if ((RetVal = m_oSections.GetHead()) != NULL)
	{
		/* Loop around until a subsection is found that matches the name passed in */

		do
		{
			if (_stricmp(RetVal->m_pcName, a_pccName) == 0)
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

/**
 * Finds the next subsection belonging to this section.
 * Finds the next subsection after the one passed in.  No kind of
 * matching is performed - whatever is next in the subsection list is what
 * is returned.
 *
 * @date	Saturday 09-Feb-2013 2:12 pm, Code HQ Ehinger Tor
 * @param	a_poSection	Ptr to the section from which to search
 * @return	The next subsection after the one specified, or NULL
 *			if there are no more sections in the list
 */

CSection *CSection::FindNextSection(CSection *a_poSection)
{
	return(m_oSections.GetSucc(a_poSection));
}

/**
 * Finds the first key of a given name belonging to this section.
 * Finds the first key within this section that matches the name passed in.
 *
 * @date	Saturday 09-Feb-2013 1:36 pm, Code HQ Ehinger Tor
 * @param	a_pccName	Ptr to the name of the key for which to search
 * @return	The first key that matches the name passed in, or NULL if
 *			there are no matching sections in the list
 */

CKey *CSection::FindKey(const char *a_pccName)
{
	CKey *RetVal;

	/* Get a ptr to the first key in the section */

	if ((RetVal = m_oKeys.GetHead()) != NULL)
	{
		/* Loop around until a key is found that matches the name passed in */

		do
		{
			if (_stricmp(RetVal->m_pcName, a_pccName) == 0)
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

/**
 * Finds the next key belonging to this section.
 * Finds the next key within this section that matches the name passed in.
 * The search begins with the next key after the one passed in.  By using
 * CSection::FindKey() in conjunction with this function it is possible to
 * iterate through all of the keys of a given name that are present
 *
 * @date	Saturday 09-Feb-2013 1:54 pm, Code HQ Ehinger Tor
 * @param	a_poKey		Ptr to the key from which to search
 * @param	a_pccName	Ptr to the name of the key for which to search
 * @return	The next key that matches the name passed in, or NULL if
 *			there are no matching sections in the list
 */

CKey *CSection::FindNextKey(CKey *a_poKey, const char *a_pccName)
{
	CKey *RetVal;

	/* Start searching at the key passed in */

	RetVal = a_poKey;

	/* Get a ptr to the next key in the list and loop around checking to see if it */
	/* matches the one passed in */

	while ((RetVal = m_oKeys.GetSucc(RetVal)) != NULL)
	{
		if (_stricmp(RetVal->m_pcName, a_pccName) == 0)
		{
			break;
		}
	}

	return(RetVal);
}

/**
 * Opens a configuration file for parsing.
 * Opens a configuration file, reads its contents into memory and parses it in preparation
 * for querying by the section, group and key querying functions.
 *
 * @date	Wednesday 22-Apr-1998 9:33 pm
 * @param	a_pccFileName	Name of configuration file to open
 * @return	KErrNone if successful
 * @return	KErrNoMemory if there was not enough memory to read the file
 * @return	KErrEof if the file could be opened but not completely read
 * @return	Any error from Utils::GetFileInfo()
 * @return	Any error from RFile::open()
 */

TInt RConfigFile::open(const char *a_pccFileName)
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

			if ((RetVal = File.open(a_pccFileName, EFileRead)) == KErrNone)
			{
				if (File.read((unsigned char *) m_pcBuffer, m_iBufferSize) == m_iBufferSize)
				{
					/* And parse it into lists of sections, subsections, groups and keys */

					RetVal = Parse();
				}
				else
				{
					RetVal = KErrEof;
				}

				File.close();
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
		close();
	}

	return(RetVal);
}

/**
 * Closes a configuration file.
 * Frees any resources associated with the configuration file.  Because this class is
 * an "R" class, this function must be explicitly called as there is no destructor.
 *
 * @date	Wednesday 22-Apr-1998 9:35 pm
 */

void RConfigFile::close()
{
	CSection *Section;

	delete [] m_pcBuffer;
	m_pcBuffer = NULL;

	/* Delete all of the sections found in the configuration file */

	while ((Section = m_oSections.remHead()) != NULL)
	{
		delete Section;
	}
}

/**
 * Searches for the first section of the given name.
 * Finds the first top level section within this configuration file that matches the name
 * the name passed in.  The returned CSection instance can then be used for searching for
 * subsections, groups and key::value pairs.
 *
 * @date	Saturday 09-Feb-2013 1:04 pm, Code HQ Ehinger Tor
 * @param	a_pccName	Ptr to the name of the section for which to search
 * @return	The first section that matches the name passed in, or NULL
 *			if there are no matching sections in the configuration file
 */

CSection *RConfigFile::FindSection(const char *a_pccName)
{
	CSection *RetVal;

	if ((RetVal = m_oSections.GetHead()) != NULL)
	{
		do
		{
			if (_stricmp(RetVal->m_pcName, a_pccName) == 0)
			{
				break;
			}

			RetVal = m_oSections.GetSucc(RetVal);
		}
		while (RetVal);
	}

	return(RetVal);
}

/**
 * Finds an integral key::value pair in a specified section::subsection.
 * Retrieves the value of an integer belonging to a user specified key, in a
 * specified section and subsection.  Section, subsection and key names are case
 * insensitive.  This is a convenience function that can be used when you know
 * the name of the key and the section::subsection in which it resides, and do
 * not wish or need to search through sections or groups to retrieve it.
 *
 * @date	Wednesday 29-Apr-1998 11:08 pm
 * @param	a_pccSectionName	Ptr to name of section the key is in
 * @param	a_pccSubSectionName	Ptr to name of subsection the key is in in
 * @param	a_pccKeyName		Ptr to name of key to read
 * @param	a_piResult			Ptr to integer variable into which to place
 *								the retrieved key value
 * @return	KErrNone if successful
 * @return	KErrNoMemory if there was not enough memory to extract the key's value
 * @return	KErrNotFound if a key with the specified name could not be found
 * @return	KErrCorrupt if the key was found but its value was not a valid number
 */

TInt RConfigFile::GetInteger(const char *a_pccSectionName, const char *a_pccSubSectionName,
	const char *a_pccKeyName, TInt *a_piResult)
{
	char *ConfigString;
	TInt RetVal;

	/* Get the string associated with the requested setting */

	GetString(a_pccSectionName, a_pccSubSectionName, a_pccKeyName, ConfigString);

	/* If the string was obtained successfully, convert it to an integer and */
	/* and save it, and free the string */

	if (ConfigString)
	{
		RetVal = Utils::StringToInt(ConfigString, a_piResult);

		delete [] ConfigString;
	}
	else
	{
		RetVal = KErrNotFound;
	}

	return(RetVal);
}

/**
 * Finds a string key::value pair in a specified section::subsection.
 * Retrieves the value of a string belonging to a user specified key, in a
 * specified section and subsection.  Section, subsection and key names are case
 * insensitive.  In the case where no key is found or an error occurs, the contents
 * of a_rpcResult will be set to NULL.  This is a convenience function that can be
 * used when you know the name of the key and the section::subsection in which it
 * resides, and do not wish or need to search through sections or groups to retrieve it.
 *
 * @date	Wednesday 22-Apr-1998 9:31 pm
 * @param	a_pccSectionName	Name of section from which to read
 * @param	a_pccSubSectionName	Name of subsection from which to read
 * @param	a_pccKeyName		Name of key to be read
 * @param	a_rpcResult			Reference to ptr into which to place ptr to retrieved string
 * @return	KErrNone if successful
 * @return	KErrNoMemory if there was not enough memory to extract the key's value
 * @return	KErrNotFound if the key was not found due to the key being missing or
 *						 the requested section and/or subsection not being found
 */

TInt RConfigFile::GetString(const char *a_pccSectionName, const char *a_pccSubSectionName,
	const char *a_pccKeyName, char *&a_rpcResult)
{
	char *Buffer, *Heading;
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
	KeyNameLength = (TInt) strlen(a_pccKeyName);
	SectionNameLength = (TInt) strlen(a_pccSectionName);
	SubSectionNameLength = (TInt) strlen(a_pccSubSectionName);

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

		Heading = Utils::StripDags(Buffer, &LFIndex);

		/* If it is a comment then simply skip this entire line */

		if (Heading[0] != ';')
		{
			/* If this is a section marker then see if it is the one we are interested */
			/* in.  If a section or subsection has already been found then abort processing */
			/* as duplicate sections of the same name are not supported and we have not */
			/* found the key we are looking for in the currently active section/subsection */

			if ((Heading[0] == '(') && (Heading[LFIndex - 1] == ')'))
			{
				if (!(FoundSubSection))
				{
					if (!(FoundSection))
					{
						/* Is this the section we are after?  Only do a comparison if the token */
						/* found is the same length as the section name (taking into account the */
						/* '(' and ')' characters surrounding the token) */

						if (SectionNameLength == (LFIndex - 2))
						{
							FoundSection = (_strnicmp(&Heading[1], a_pccSectionName, SectionNameLength) == 0);
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
				if ((Heading[0] == '[') && (Heading[LFIndex - 1] == ']'))
				{
					if (!(FoundSubSection))
					{
						/* Is this the subsection we are after?  Only do a comparison if the token */
						/* found is the same length as the subsection name (taking into account the */
						/* '[' and ']' characters surrounding the token) */

						if (SubSectionNameLength == (LFIndex - 2))
						{
							FoundSubSection = (_strnicmp(&Heading[1], a_pccSubSectionName, SubSectionNameLength) == 0);
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
					/* Initialise a TLex to parse the current line for tokens and configure it to treat */
					/* spaces, tabs and the = sign as white space.  This allows the user flexibility in */
					/* how they layout the .ini file */

					TLex Lex(Buffer, Index);

					Lex.SetWhitespace(" \t=");

					/* Get the first token from the line */

					if ((Token = Lex.NextToken(&TokenLength)) != NULL)
					{
						/* Is this the key we are after?  Only do a comparison if the token found is */
						/* the same length as the target key, to avoid finding keys that are substrings */
						/* of the token.  ie.  We don't want "Short" to match "ShortKey" */

						if ((TokenLength == KeyNameLength) && (_strnicmp(Token, a_pccKeyName, KeyNameLength) == 0))
						{
							/* Yep!  Ket the key's value, skipping over any whitespace and = signs */
							/* between the key and its value */

							if ((Token = Lex.NextToken(&TokenLength)) != NULL)
							{
								/* Duplicate the token into an allocated buffer */

								if ((a_rpcResult = Utils::DuplicateString(Token, TokenLength)) != NULL)
								{
									RetVal = KErrNone;

									break;
								}
								else
								{
									Utils::info("RConfigFile::GetString() => Not enough memory to allocate string");

									RetVal = KErrNoMemory;
								}
							}
						}
					}
				}
			}
		}

		/* Update the buffer variables so they point to the start of the next line, for the */
		/* next iteration of the loop */

		++Index;
		Buffer += Index;
		BufferSize -= Index;
	}

	return(RetVal);
}

/**
 * Parses the sections, subsections, groups and key::value pairs in a configuration file.
 * This function iterates through a configuration file that has already been loaded into memory
 * by RConfigFile::open() and creates a tree in memory that represents all of the sections,
 * subsections, groups and key = value pairs that are present in the file.  After this, client
 * code is able to call functions such as RConfigFile::FindSection() and RConfigFile::FindKey()
 * to search for desired key = value pairs
 *
 * @date	Saturday 09-Feb-2013 11:00 am, Code HQ Ehinger Tor
 * @return	KErrNone if successful
 * @return	KErrNoMemory if there wasn't enough memory to allocate all nodes
 */

TInt RConfigFile::Parse()
{
	char *Buffer, *Heading;
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

		Heading = Utils::StripDags(Buffer, &LFIndex);

		/* If it is a comment then simply skip this entire line */

		if (Heading[0] != ';')
		{
			/* If this is a valid section marker then allocate a CSection instance for it */
			/* and add it to the list of top level sections */

			if ((Heading[0] == '(') && (Heading[LFIndex - 1] == ')'))
			{
				if ((NewSection = CSection::New(&Heading[1], (LFIndex - 2))) != NULL)
				{
					/* Make this the current section and indicate that there are no longer */
					/* any currently active subsections or groups.  Otherwise any newly found */
					/* groups or key would be added to the incorrect subsection */

					Section = NewSection;
					SubSection = Group = NULL;

					/* And add the section to the list of top level sections */

					m_oSections.addTail(NewSection);
				}
				else
				{
					break;
				}
			}

			/* If this is a valid subsection marker then allocate a CSection instance for it */
			/* and add it to the list of subsections in the current top level section */

			else if ((Heading[0] == '[') && (Heading[LFIndex - 1] == ']'))
			{
				if ((NewSection = CSection::New(&Heading[1], (LFIndex - 2))) != NULL)
				{
					/* Only use the subsection if it is within a section */

					if (Section)
					{
						/* Indicate that this is now the currently active subsection and that */
						/* there is no longer an active group */

						SubSection = NewSection;
						Group = NULL;

						/* And add the subsection to the current section's list of subsections */

						Section->m_oSections.addTail(NewSection);
					}

					/* Otherwise the subsection is invalid so discard it and display a warning */

					else
					{
						Utils::info("Discarding orphan subsection %s", NewSection->m_pcName);

						delete NewSection;
					}
				}
				else
				{
					break;
				}
			}

			/* If this is a valid group marker then allocate a CSection instance for it */
			/* and add it to the list of subsections in the current subsection */

			else if ((Heading[0] == '{') && (Heading[LFIndex - 1] == '}'))
			{
				if ((NewSection = CSection::New(&Heading[1], (LFIndex - 2))) != NULL)
				{
					/* Only use the subsection if it is within a subsection */

					if (SubSection)
					{
						/* Indicate that this is now the currently active group */

						Group = NewSection;

						/* And add the group to the current subsection's list of groups */

						SubSection->m_oSections.addTail(NewSection);
					}

					/* Otherwise the group is invalid so discard it and display a warning */

					else
					{
						Utils::info("Discarding orphan group %s", NewSection->m_pcName);

						delete NewSection;
					}
				}
				else
				{
					break;
				}
			}
			else
			{
				/* Initialise a TLex to parse the current line for tokens and configure it to treat */
				/* spaces, tabs and the = sign as white space.  This allows the user flexibility in */
				/* how they layout the .ini file */

				TLex Lex(Buffer, LFIndex);

				Lex.SetWhitespace(" \t=");

				/* Get the first token from the line */

				if ((Token = Lex.NextToken(&TokenLength)) != NULL)
				{
					/* If this is a valid key and value pair CKey instance for it */

					if ((ValueToken = Lex.NextToken(&ValueTokenLength)) != NULL)
					{
						/* Recalculate the length of the value, in case it contains any whitespace.  In this */
						/* case we want to include the whitespace and any text after it, but it will have been */
						/* stripped out of the value token by TLex, which is also using it as a key = value */
						/* separator */

						ValueTokenLength = (TInt) (LFIndex - (ValueToken - Token));

						if ((Key = CKey::New(Token, TokenLength, ValueToken, ValueTokenLength)) != NULL)
						{
							/* Only use the key and value if it is within a subsection or a group */

							if ((SubSection) || (Group))
							{
								/* Add the new key to the current group, if there is one.  Otherwise add */
								/* it to the current subsection */

								if (Group)
								{
									Group->m_oKeys.addTail(Key);
								}
								else
								{
									SubSection->m_oKeys.addTail(Key);
								}
							}

							/* Otherwise the key is invalid so discard it and display a warning */

							else
							{
								Utils::info("Discarding orphan key %s", Key->m_pcName);

								delete Key;
							}
						}
						else
						{
							break;
						}
					}
				}
			}
		}

		/* Update the buffer variables so they point to the start of the next line, for the */
		/* next iteration of the loop */

		++Index;
		Buffer += Index;
		BufferSize -= Index;
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
