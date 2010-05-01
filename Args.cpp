
#include "StdFuncs.h"

#ifdef __amigaos4__

#include <proto/icon.h>
#include <workbench/startup.h>

#else /* ! __amigaos4__ */

#include <stdio.h>

#endif /* ! __amiga_os4 */

#include <string.h>
#include "Args.h"

#ifndef __amigaos4__

// TODO: CAW - Comment all through here

/* Written: Thursday 16-Jul-2009 6:37 am */

static char *ExtractOption(const char *a_pccTemplate, int a_iIndex, char *a_pcType)
{
	char Char, *RetVal;
	int Offset, StartOffset, EndOffset;

	RetVal = NULL;
	Offset = StartOffset = EndOffset = 0;
	*a_pcType = '\0';

	while (a_iIndex >= 0)
	{
		for ( ; ; )
		{
			Char = a_pccTemplate[Offset];

			if ((Char == '\0') || (Char == ','))
			{
				if (a_iIndex > 0)
				{
					--a_iIndex;
					StartOffset = EndOffset = (Offset + 1);
				}
				else
				{
					a_iIndex = -1;

					break;
				}
			}
			else
			{
				if (Char == '/')
				{
					EndOffset = Offset;
					*a_pcType = a_pccTemplate[Offset + 1];
				}
			}

			++Offset;

			if (Char == '\0')
			{
				a_iIndex = -1;

				break;
			}
		}
	}

	if (StartOffset < EndOffset)
	{
		// TODO: CAW - I don't like this because it doesn't differentiate between out of memory and other failures
		if ((RetVal = new char[(EndOffset - StartOffset) + 1]) != NULL)
		{
			for (Offset = 0; StartOffset < EndOffset; ++StartOffset)
			{
				RetVal[Offset++] = a_pccTemplate[StartOffset];
			}

			RetVal[Offset] = '\0';
		}
	}

	return(RetVal);
}

#endif /* ! __amigaos4__ */

/* Written: Sunday 04-Nov-2007 11:48 am */

RArgs::RArgs()
{
	m_pcCommandLine = m_pcProjectFileName = NULL;
	m_iNumArgs = 0;
	m_plArgs = NULL;
	m_poRDArgs = m_poTTRDArgs = NULL;
}

/* Written: Sunday 04-Nov-2007 11:51 am */

int RArgs::Open(const char *a_pccTemplate, int a_iNumOptions, const char *a_pccArgV[], int a_iArgC)
{
	int RetVal;

	/* Assume failure */

	RetVal = KErrNoMemory;

	/* Allocate an array of LONGs into which ptrs to arguments can be placed */

	if ((m_plArgs = new LONG[a_iNumOptions]) != NULL)
	{
		/* Save the # of arguments for l8r */

		m_iNumArgs = a_iNumOptions;

		/* Now read the arguments, according to the argument template */

#ifdef __amigaos4__

		(void) a_pccArgV;
		(void) a_iArgC;

		if ((m_poRDArgs = IDOS->ReadArgs(a_pccTemplate, m_plArgs, NULL)) != NULL)
		{
			RetVal = KErrNone;
		}
		else
		{
			Utils::Info("RArgs::Open() => Unable to read command line arguments");
		}

#else /* ! __amigaos4__ */

		char **ArgV, *OptionName, Type;
		int Arg, Index;

		if ((a_iArgC == 2) && (*a_pccArgV[1] == '?'))
		{
			// TODO: CAW - Emulate real Amiga OS behaviour
			printf("%s\n", a_pccTemplate);
		}
		else
		{
			if ((ArgV = new char *[a_iArgC]) != NULL)
			{
				RetVal = KErrNone;
				memset(m_plArgs, 0, (sizeof(LONG) * a_iNumOptions));
				memcpy(ArgV, a_pccArgV, (sizeof(char *) * a_iArgC));

				for (Index = 0; Index < a_iNumOptions; ++Index)
				{
					if ((OptionName = ExtractOption(a_pccTemplate, Index, &Type)) != NULL)
					{
						if (Type == 'S')
						{
							for (Arg = 1; Arg < a_iArgC; ++Arg)
							{
								if ((ArgV[Arg]) && (!(stricmp(ArgV[Arg], OptionName))))
								{
									m_plArgs[Index] = (LONG) ArgV[Arg];
									ArgV[Arg] = NULL;

									break;
								}
							}
						}

						delete [] OptionName;
					}
				}

				for (Index = 0; Index < a_iNumOptions; ++Index)
				{
					if ((OptionName = ExtractOption(a_pccTemplate, Index, &Type)) != NULL)
					{
						if (Type == 'A')
						{
							for (Arg = 1; Arg < a_iArgC; ++Arg)
							{
								if (ArgV[Arg])
								{
									m_plArgs[Index] = (LONG) ArgV[Arg];
									ArgV[Arg] = NULL;

									break;
								}
							}
						}

						delete [] OptionName;
					}
				}

				delete [] ArgV;
			}
			else
			{
				Utils::Info("RDArgs::Open() => Unable to allocate buffe for arguments");
			}
		}

#endif /* ! __amigaos4__ */

	}
	else
	{
		Utils::Info("RArgs::Open() => Unable to allocate array for arguments");
	}

	/* If anything failed, clean up */

	if (RetVal != KErrNone)
	{
		Close();
	}

	return(RetVal);
}

/* Written: Saturday 10-Jan-2009 3:12 pm */

int RArgs::Open(const char *a_pccTemplate, int a_iNumOptions, const struct WBStartup *a_poWBStartup)
{
	int RetVal;

#ifdef __amigaos4__

	bool Directory, FoundEquals;
	char *Source, *Dest;
	int Index, Length;
	STRPTR *ToolTypes;
	struct DiskObject *DiskObject;
	struct RDArgs *RDArgs;
	struct WBArg *WBArg;

#else /* ! __amigaos4__ */

	(void) a_pccTemplate;
	(void) a_poWBStartup;

#endif /* ! __amigaos4__ */

	/* Assume failure */

	RetVal = KErrNoMemory;

	/* Allocate an array of LONGs into which ptrs to arguments can be placed */

	if ((m_plArgs = new LONG[a_iNumOptions]) != NULL)
	{
		/* Save the # of arguments for l8r */

		m_iNumArgs = a_iNumOptions;

		/* Get some information about the .info file associated with the WBStartup message passed in */

#ifdef __amigaos4__

		WBArg = a_poWBStartup->sm_ArgList;

		if ((DiskObject = IIcon->GetDiskObject(WBArg->wa_Name)) != NULL)
		{
			/* If there are any tooltypes defined in the icon, copy them into a big string that can be */
			/* used as an emulated CLI argument list and parsed by IDOS->ReadArgs() */

			if (DiskObject->do_ToolTypes)
			{
				/* Iterate through all of the tooltypes and determine their length.  This will be used */
				/* to allocate a NULL terminated buffer large enough to copy them into which will emulate */
				/* a CLI argument list.  Each argument will have an extra 3 chars allocated to enclose it */
				/* in '"' characters and to put a space between it and the following argument */

				Index = 0;
				Length = 1;
				ToolTypes = DiskObject->do_ToolTypes;

				while ((ToolTypes[Index]) && (*ToolTypes[Index]))
				{
					Length += (strlen(ToolTypes[Index]) + 3);
					++Index;
				}

				/* We know the length required, so allocate a buffer of that size */

				if ((m_pcCommandLine = new char[Length]) != NULL)
				{
					/* Iterate through all of the tooltypes and copy them into the buffer */

					Index = 0;

					while ((ToolTypes[Index]) && (*ToolTypes[Index]))
					{
						FoundEquals = false;
						Source = ToolTypes[Index];
						Dest = (m_pcCommandLine + strlen(m_pcCommandLine));

						/* Copy the chars of the tooltype into the buffer.  If an '=' is found */
						/* then enclose the text following it with " in case it contains spaces */
						/* so that it is treated as a single argument and not broken up by */
						/* IDOS->ReadArgs() */

						while (*Source)
						{
							*Dest++ = *Source;

							if (*Source++ == '=')
							{
								FoundEquals = true;
								*Dest++ = '"';
							}
						}

						/* If a '=' was found then terminate the string with a " */

						if (FoundEquals)
						{
							*Dest++ = '"';
						}

						/* Append a space after the argument and NULL terminate the entire string */

						*Dest++ = ' ';
						*Dest++ = '\0';

						/* And point to the next entry */

						++Index;
					}

					/* Now we have a buffer that contains a CLI argument list, something like: */
					/* FILENAME="blah.mp3" CX_POPUP="no" */
					/* So allocate a DOS RDArgs structure that can be used to hold a pointer to */
					/* the argument list, and use IDOS->RDArgs() to parse it as though it was a */
					/* real CLI argument list */

					RDArgs = (struct RDArgs *) IDOS->AllocDosObjectTags(DOS_RDARGS, TAG_DONE);

					if (RDArgs)
					{
						/* Setup the CSource structure that represents the CLI argument list.  This */
						/* will then be used by IDOS->ReadArgs() instead of the real argument list */
						/* (which in this case doesn't exist seeing that we are running from Workbench */

						RDArgs->RDA_Source.CS_Buffer = m_pcCommandLine;
						RDArgs->RDA_Source.CS_Length = Length;
						RDArgs->RDA_Source.CS_CurChr = 0;

						/* Now call IDOS->ReadArgs() to parse the CLI argument list.  This will result */
						/* in the arguments in m_plArgs being populated just as though we were run from */
						/* the CLI and the client software will neither know nor care that we have been */
						/* run from Workbench! */

						if ((m_poTTRDArgs = IDOS->ReadArgs(a_pccTemplate, m_plArgs, RDArgs)) != NULL)
						{
							RetVal = KErrNone;

							/* The RArgs class supports the user launching the application by double clicking */
							/* on a project icon, or by shift clicking on a single project icon and the tool */
							/* icon for the associated program.  If this has happened then determine the fully */
							/* qualified filename of the file associated with the project icon clicked on */

							if (a_poWBStartup->sm_NumArgs == 2)
							{
								// TODO: CAW - This shouldn't be hard coded and Directory should be File
								char Path[1024];

								if (Utils::FullNameFromWBArg(Path, (a_poWBStartup->sm_ArgList + 1), &Directory))
								{
									/* If it is a file and not a directory (which are not currently supported) */
									/* then allocated a buffer and copy the fully qualified filename for l8r use */

									if (!(Directory))
									{
										if ((m_pcProjectFileName = new char[strlen(Path) + 1]) != NULL)
										{
											strcpy(m_pcProjectFileName, Path);
											IDOS->Printf("%s\n", Path);
										}
										else
										{
											Utils::Info("RDArgs::Read() => Out of memory");
										}
									}
								}
							}
						}
						else
						{
							Utils::Info("RArgs::Open() => Unable to read command line arguments");

							IDOS->FreeDosObject(DOS_RDARGS, RDArgs);
						}
					}
					else
					{
						Utils::Info("RDArgs::Read() => Unable to allocate DOS RDArgs structure");
					}
				}
				else
				{
					Utils::Info("RDArgs::Read() => Out of memory");
				}
			}

			/* The icon contains no tooltypes, but this is not an error.  It just means there are */
			/* no startup arguments */

			else
			{
				RetVal = KErrNone;

				Utils::Info("RArgs::Read() => Icon contains no tooltypes");
			}

			IIcon->FreeDiskObject(DiskObject);
		}
		else
		{
			Utils::Info("RArgs::Read() => Unable to open icon for object \"%s\"", WBArg->wa_Name);
		}

#endif /* __amigaos4__ */

	}
	else
	{
		Utils::Info("RArgs::Open() => Unable to allocate array for arguments");
	}

	/* If anything failed, clean up */

	if (RetVal != KErrNone)
	{
		Close();
	}

	return(RetVal);
}

/* Written: Sunday 04-Nov-2007 11:52 am */

void RArgs::Close()
{
	/* Delete the emulated command line arguments buffer */

	delete [] m_pcCommandLine;
	m_pcCommandLine = NULL;

	/* Delete the buffer used for holding the project filename */

	delete [] m_pcProjectFileName;
	m_pcProjectFileName = NULL;

	/* And the array used for ptrs to arguments */

	delete [] m_plArgs;
	m_plArgs = NULL;

#ifdef __amigaos4__

	/* Free the tooltype argument structure, if it exists */

	if (m_poTTRDArgs)
	{
		IDOS->FreeArgs(m_poTTRDArgs);
		m_poTTRDArgs = NULL;
	}

	/* Free the argument structure, if it exists */

	if (m_poRDArgs)
	{
		IDOS->FreeArgs(m_poRDArgs);
		m_poRDArgs = NULL;
	}

#endif /* __amigaos4__ */

	/* Ensure that everything is back to the exact state it was in before Open() was called */

	m_iNumArgs = 0;
}

/* Written: Sunday 04-Nov-2007 12:12 pm */

int RArgs::Count()
{
	return(m_iNumArgs);
}

/* Written: Tuesday 13-Jan-2009 7:19 am */

const char *RArgs::ProjectFileName()
{
	return(m_pcProjectFileName);
}

/* Written: Sunday 04-Nov-2007 12:17 pm */

int RArgs::Valid()
{
	int Index, RetVal;

	/* Assume no arguments were passed in */

	RetVal = 0;

	/* Iterate through the argument ptrs and count how many are valid */

	for (Index = 0; Index < m_iNumArgs; ++Index)
	{
		if (m_plArgs[Index])
		{
			++RetVal;
		}
	}

	return(RetVal);
}

/* Written: Sunday 04-Nov-2007 11:57 am */

const char *RArgs::operator[](int a_iIndex)
{
	ASSERTM(((a_iIndex >= 0) && (a_iIndex < m_iNumArgs)), "RArgs::operator[]() => a_iIndex is out of range");

	return((const char *) m_plArgs[a_iIndex]);
}
