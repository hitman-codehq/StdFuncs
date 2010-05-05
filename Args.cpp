
#include "StdFuncs.h"

#ifdef __amigaos4__

#include <proto/icon.h>
#include <workbench/startup.h>

#else /* ! __amigaos4__ */

#include <stdio.h>

#endif /* ! __amiga_os4 */

#include <string.h>
#include "Args.h"

/* Written: Sunday 04-Nov-2007 11:48 am */

RArgs::RArgs()
{
	m_pcCommandLine = m_pcProjectFileName = NULL;
	m_iNumArgs = 0;
	m_plArgs = NULL;
	m_poRDArgs = m_poTTRDArgs = NULL;
}

/* Written: Sunday 04-Nov-2007 11:51 am */

TInt RArgs::Open(const char *a_pccTemplate, TInt a_iNumOptions, const char *a_pccArgV[], TInt a_iArgC)
{
	const char **ArgV;
	TInt ArgC, RetVal;

	/* Allocate an array of LONGs into which ptrs to arguments can be placed */

	if ((m_plArgs = new LONG[a_iNumOptions]) != NULL)
	{
		/* Save the # of arguments for l8r */

		m_iNumArgs = a_iNumOptions;

		/* Now read the arguments, according to the argument template */

#ifdef __amigaos4__

		(void) a_pccArgV;
		(void) a_iArgC;

		/* For Amiga OS we can let dos.library do the hard work for us */

		if ((m_poRDArgs = IDOS->ReadArgs(a_pccTemplate, m_plArgs, NULL)) != NULL)
		{
			RetVal = KErrNone;
		}
		else
		{
			Utils::Info("RArgs::Open() => Unable to read command line arguments");
		}

#else /* ! __amigaos4__ */

		/* If the user has requested to display the template then do so and request input and parse that instead */
		/* of what was passed in on the command line */

		if ((a_iArgC == 2) && (*a_pccArgV[1] == '?'))
		{
			if ((ArgV = InputCommandLine(a_pccTemplate, &ArgC)) != NULL)
			{
				/* Command line has been input so put the executable's name in the first argument slot and parse */
				/* it for arguments */

				ArgV[0] = a_pccArgV[0];
				RetVal = ReadArgs(a_pccTemplate, a_iNumOptions, ArgV, ArgC);
				delete [] ArgV;
			}
			else
			{
				RetVal = KErrNoMemory;
			}
		}
		else
		{
			RetVal = ReadArgs(a_pccTemplate, a_iNumOptions, a_pccArgV, a_iArgC);
		}

#endif /* ! __amigaos4__ */

	}
	else
	{
		RetVal = KErrNoMemory;

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

TInt RArgs::Open(const char *a_pccTemplate, TInt a_iNumOptions, const struct WBStartup *a_poWBStartup)
{
	TInt RetVal;

#ifdef __amigaos4__

	char *Source, *Dest;
	TBool Directory, FoundEquals;
	TInt Index, Length;
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
						FoundEquals = EFalse;
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
								FoundEquals = ETrue;
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

TInt RArgs::Count()
{
	return(m_iNumArgs);
}

#ifndef __amigaos4__

/* Written: Thursday 16-Jul-2009 6:37 am */

TInt RArgs::ExtractOption(const char *a_pccTemplate, TInt *a_piOffset, char **a_ppcOption, char *a_pcType)
{
	char Char, *Option;
	TInt Offset, StartOffset, EndOffset, RetVal;

	/* Assume the option wasn't extracted successfully */

	RetVal = KErrNotFound;
	*a_ppcOption = NULL;
	*a_pcType = '\0';

	/* Iterate through the template, starting at the offset passed in, and look for the type separator for */
	/* the current option and the ',' that separates this option from the next one */

	Offset = StartOffset = EndOffset = *a_piOffset;

	for ( ; ; )
	{
		Char = a_pccTemplate[Offset];

		/* If we are at the end of the template or the current option then break out of the loop */

		if ((Char == '\0') || (Char == ','))
		{
			++Offset;

			break;
		}

		/* Otherwise if we have found the type separator then save its offset so that the option can be */
		/* extracted later, and save the type itself */

		else if (Char == '/')
		{
			EndOffset = Offset;
			*a_pcType = a_pccTemplate[Offset + 1];
		}

		++Offset;
	}

	/* Update the user's offset value so that we can continue on from the same place next time */
	/* we are called */

	*a_piOffset = Offset;

	/* If we have found a keyword terminated by a '/' then we have found the requested option so */
	/* allocate a buffer for it and extract the option so it can be returned.  In this case the type */
	/* of the option will already have been returned by the code above */

	if (StartOffset < EndOffset)
	{
		if ((*a_ppcOption = Option = new char[(EndOffset - StartOffset) + 1]) != NULL)
		{
			/* Indicate success */

			RetVal = KErrNone;

			/* And copy the found option into the allocated buffer */

			for (Offset = 0; StartOffset < EndOffset; ++StartOffset)
			{
				Option[Offset++] = a_pccTemplate[StartOffset];
			}

			Option[Offset] = '\0';
		}
		else
		{
			RetVal = KErrNoMemory;
		}
	}

	return(RetVal);
}

#endif /* ! __amigaos4__ */

/* Written: Tuesday 13-Jan-2009 7:19 am */

const char *RArgs::ProjectFileName()
{
	return(m_pcProjectFileName);
}

#ifndef __amigaos4__

/* Written: Sunday 03-05-2010 9:38 am */

const char **RArgs::InputCommandLine(const char *a_pccTemplate, TInt *a_piArgC)
{
	char *Arg, Buffer[1024], Char;
	const char **RetVal;
	TBool CharFound;
	TInt Index, Offset, NumArgs, Source, Dest;

	/* Display the template a la Amiga OS and prompt the user for input */

	printf("%s\n", a_pccTemplate);
	gets(Buffer);

	/* Iterate through the command line the user just entered and strip out any extranneous spaces */
	/* in the input.  It is easier to pull out the individual tokens if they are separated by just */
	/* one space.  We will also count the number of tokens that are input in this loop */

	CharFound = EFalse;
	Dest = 0;
	NumArgs = 2;

	for (Source = 0; Buffer[Source]; ++Source)
	{
		Char = Buffer[Source];

		/* If the current character is a space then copy it only if the previous character was not */
		/* also a space */

		if (Char == ' ')
		{
			if (!(CharFound))
			{
				CharFound = ETrue;
				Buffer[Dest++] = Buffer[Source];
				++NumArgs;
			}
		}

		/* The current character is not a space so just copy it and indicate this fact */

		else
		{
			CharFound = EFalse;
			Buffer[Dest++] = Buffer[Source];
		}
	}

	Buffer[Dest] = '\0';

	/* Allocate an array of ptrs to strings, that can be used as an ArgV style array to hold ptrs */
	/* to the tokens entered above */

	if ((RetVal = new const char *[NumArgs]) != NULL)
	{
		/* Iterate through the command line entered and extract each token, NULL terminating it and */
		/* putting a ptr to it in the ArgV array */

		Offset = 0;

		for (Index = 1; Index < NumArgs; ++Index)
		{
			if ((Arg = strtok(&Buffer[Offset], " ")) != NULL)
			{
				RetVal[Index] = Arg;
				Offset += (strlen(Arg) + 1);
			}
			else
			{
				break;
			}
		}

		/* And save the number of tokens actually extracted */

		*a_piArgC = Index;
	}
	else
	{
		Utils::Info("RDArgs::InputCommandLine() => Out of memory");
	}

	return(RetVal);
}

/* Written: Saturday 02-05-2010 8:52 am */

TInt RArgs::ReadArgs(const char *a_pccTemplate, TInt a_iNumOptions, const char *a_pccArgV[], TInt a_iArgC)
{
	char **ArgV, *OptionName, Type;
	TInt Arg, Index, Offset, RetVal;

	/* Assume success */

	RetVal = KErrNone;

	/* Allocate a buffer large enouth to hold the arguments passed in from the command line.  This */
	/* is used to keep track of which arguments have been found already so that /A options can be */
	/* automatically filled in after scanning the other options */

	if ((ArgV = new char *[a_iArgC]) != NULL)
	{
		memcpy(ArgV, a_pccArgV, (sizeof(char *) * a_iArgC));
		memset(m_plArgs, 0, (sizeof(LONG) * a_iNumOptions));
		Offset = 0;

		/* Iterate through the template and extract all S options */

		for (Index = 0; Index < a_iNumOptions; ++Index)
		{
			if ((RetVal = ExtractOption(a_pccTemplate, &Offset, &OptionName, &Type)) == KErrNone)
			{
				if (Type == 'S')
				{
					/* Now scan through the arguments passed in and see if this option has been specified */

					for (Arg = 1; Arg < a_iArgC; ++Arg)
					{
						if ((ArgV[Arg]) && (!(stricmp(ArgV[Arg], OptionName))))
						{
							/* Found the option!  Copy it into the args array so that queries by client code */
							/* return that it has been found and set the temporary copy to NULL so that only */
							/* unused arguments are left after S option extraction */

							m_plArgs[Index] = (LONG) ArgV[Arg];
							ArgV[Arg] = NULL;

							break;
						}
					}
				}

				delete [] OptionName;
			}
			else
			{
				break;
			}
		}

		/* If no error occurred, now we have to iterate through the template again, this time looking for A */
		/* options and using whatever arguments are leftover on the command line as their values */

		if (RetVal == KErrNone)
		{
			Offset = 0;

			for (Index = 0; Index < a_iNumOptions; ++Index)
			{
				if ((RetVal = ExtractOption(a_pccTemplate, &Offset, &OptionName, &Type)) == KErrNone)
				{
					if (Type == 'A')
					{
						/* Now scan through the arguments passed in looking for an unused one */

						for (Arg = 1; Arg < a_iArgC; ++Arg)
						{
							if (ArgV[Arg])
							{
								/* Found an unused argument so copy this into the args array so that it represents */
								/* the current A option when queried by client code.  Set the temporary copy to NULL */
								/* so taht it is not reused again for the next A option */

								m_plArgs[Index] = (LONG) ArgV[Arg];
								ArgV[Arg] = NULL;

								break;
							}
						}
					}

					delete [] OptionName;
				}
				else
				{
					break;
				}
			}
		}

		delete [] ArgV;
	}
	else
	{
		RetVal = KErrNoMemory;

		Utils::Info("RDArgs::Open() => Unable to allocate buffer for arguments");
	}

	return(RetVal);
}

#endif /* ! __amigaos4__ */

/* Written: Sunday 04-Nov-2007 12:17 pm */

TInt RArgs::Valid()
{
	TInt Index, RetVal;

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

const char *RArgs::operator[](TInt a_iIndex)
{
	ASSERTM(((a_iIndex >= 0) && (a_iIndex < m_iNumArgs)), "RArgs::operator[]() => a_iIndex is out of range");

	return((const char *) m_plArgs[a_iIndex]);
}
