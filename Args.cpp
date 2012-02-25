
#include "StdFuncs.h"
#include "Lex.h"

#ifdef __amigaos4__

#include <proto/icon.h>
#include <workbench/startup.h>

#else /* ! __amigaos4__ */

#include <stdio.h>

#endif /* ! __amigaos4__ */

#include <string.h>
#include "Args.h"

/* Written: Sunday 04-Nov-2007 11:48 am */

RArgs::RArgs()
{
	m_pcBuffer = m_pcCommandLine = m_pcProjectFileName = NULL;
	m_iMagicOption = -1;
	m_iNumArgs = 0;
	m_plArgs = NULL;
	m_poRDArgs = m_poTTRDArgs = m_poInputRDArgs = NULL;
}

/* Written: Sunday 04-Nov-2007 11:51 am */

TInt RArgs::Open(const char *a_pccTemplate, TInt a_iNumOptions, const char *a_pccArgV[], TInt a_iArgC)
{
	TInt RetVal;

	/* Save the # of arguments for l8r */

	m_iNumArgs = a_iNumOptions;

	/* Allocate an array of LONGs into which ptrs to arguments can be placed. */
	/* We allocate an extra entry for the name of the executable, which will */
	/* go into m_plArgs[0] */

	if ((m_plArgs = new LONG[m_iNumArgs]) != NULL)
	{
		memset(m_plArgs, 0, (sizeof(LONG) * m_iNumArgs));

		/* Determine which if the option (if any) is the magic multi option */

		FindMagicMultiOption(a_pccTemplate, a_iNumOptions);

		/* Now read the arguments, according to the argument template */

#ifdef __amigaos4__

		(void) a_pccArgV;
		(void) a_iArgC;

		/* For Amiga OS we can let dos.library do the hard work for us */

		m_poInputRDArgs	= (struct RDArgs *) IDOS->AllocDosObjectTags(DOS_RDARGS, TAG_DONE);

		if (m_poInputRDArgs)
		{
			int Index = 0; // TODO: CAW
			int Size = 2;

			for (Index = 1; Index < a_iArgC; ++Index)
			{
				Size += (strlen(a_pccArgV[Index]) + 3); // TODO: CAW - Comments here & above
			}

			if ((m_pcBuffer = new char[Size]) != NULL)
			{
				m_pcBuffer[0] = '\0';

				for (Index = 1; Index < a_iArgC; ++Index)
				{
					if (strstr(a_pccArgV[Index], " ") > 0) // TODO: CAW - Bodgey
						strcat(m_pcBuffer, "\"");

					strcat(m_pcBuffer, a_pccArgV[Index]);

					if (strstr(a_pccArgV[Index], " ") > 0) // TODO: CAW - Bodgey
						strcat(m_pcBuffer, "\"");

					if ((Index + 1) < a_iArgC)
						strcat(m_pcBuffer, " ");
				}

				strcat(m_pcBuffer, "\n");

				m_poInputRDArgs->RDA_Source.CS_Buffer = m_pcBuffer;
				m_poInputRDArgs->RDA_Source.CS_Length = Size;

		// TODO: CAW
		if ((m_poRDArgs = IDOS->ReadArgs(a_pccTemplate, m_plArgs, m_poInputRDArgs)) != NULL)
		{
			RetVal = KErrNone;
		}
		else
		{
			// TODO: CAW - What about other errors such as ERROR_TOO_MANY_ARGS?
			if (IDOS->IoErr() == ERROR_REQUIRED_ARG_MISSING)
			{
				RetVal = KErrNotFound;
			}
			else
			{
				// TODO: CAW - What about handling ERROR_TOO_MANY_ARGS?
				RetVal = KErrNoMemory;
			}

			Utils::Info("RArgs::Open() => Unable to read command line arguments");
		}
			}
		}

#else /* ! __amigaos4__ */

		const char **ArgV;
		TInt ArgC;

		/* If the user has requested to display the template then do so and request input and parse that instead */
		/* of what was passed in on the command line */

		if ((a_iArgC == 2) && (*a_pccArgV[1] == '?'))
		{
			/* Display the template a la Amiga OS and prompt the user for input */

			// TODO: CAW - This doesn't work with WinMain() style programs so NULL termination below is temporary
			char Buffer[1024];

			Buffer[0] = '\0';
			printf("%s\n", a_pccTemplate);
			gets(Buffer);

			if ((ArgV = ExtractArguments(Buffer, &ArgC)) != NULL)
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

/* Written: Friday 04-Jun-2010 8:36 am  */
/* @param	a_pcArguments	Ptr to a string containing a list of arguments passed in */
/*							from the command line.  Note that this is a Win32 style list */
/*							as passed into WinMain() and thus the first argument is NOT */
/*							the filename */

// TODO: CAW - Call ReadArgs() directly from within here
TInt RArgs::Open(const char *a_pccTemplate, TInt a_iNumOptions, char *a_pcArguments)
{
	const char **ArgV;
	TInt ArgC, RetVal;

	/* Extract the arguments from the string into an ArgV style ptr array.  This */
	/* will result in ArgC being the number of arguments + 1 as ExtractArguments() */
	/* adds an extra (unused) entry for the filename as the first argument.  This */
	/* is required so that when we call Open() it will ignore this first argument, */
	/* for compatibility with argument lists passed into main(), which also have */
	/* the filename in argv[0]! */

	if ((ArgV = ExtractArguments(a_pcArguments, &ArgC)) != NULL)
	{
		/* And pass that array into the standard RArgs::Open() to extract the arguments */

		//ArgV[0] = "Test"; // TODO: CAW - How to obtain this?  When we fix this, it will break BUBYFU and maybe others
		RetVal = Open(a_pccTemplate, a_iNumOptions, ArgV, ArgC);
		delete [] ArgV;
	}
	else
	{
		RetVal = KErrNoMemory;

		Utils::Info("RArgs::Open() => Unable to allocate array for arguments");
	}

	return(RetVal);
}

/* Written: Saturday 10-Jan-2009 3:12 pm */

// TODO: CAW - Comment out for Win32?
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
										}
										else
										{
											Utils::Info("RDArgs::Open() => Out of memory");
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
						Utils::Info("RDArgs::Open() => Unable to allocate DOS RDArgs structure");
					}
				}
				else
				{
					Utils::Info("RDArgs::Open() => Out of memory");
				}
			}

			/* The icon contains no tooltypes, but this is not an error.  It just means there are */
			/* no startup arguments */

			else
			{
				RetVal = KErrNone;

				Utils::Info("RArgs::Open() => Icon contains no tooltypes");
			}

			IIcon->FreeDiskObject(DiskObject);
		}
		else
		{
			Utils::Info("RArgs::Open() => Unable to open icon for object \"%s\"", WBArg->wa_Name);
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

	if (m_poInputRDArgs)
	{
		IDOS->FreeDosObject(DOS_RDARGS, m_poInputRDArgs);
		m_poInputRDArgs = NULL;
	}

	delete m_pcBuffer;
	m_pcBuffer = NULL;

#endif /* __amigaos4__ */

	/* Ensure that everything is back to the exact state it was in before Open() was called */

	m_iNumArgs = 0;
}

/* Written: Sunday 04-Nov-2007 12:12 pm */

TInt RArgs::Count()
{
	return(m_iNumArgs);
}

/* Written: Saturday 29-Oct-2011 8:05 am, CodeHQ Söflingen */

TInt RArgs::CountMultiArguments()
{
	TInt RetVal;
	const char **MultiArguments;

	/* Assume no multi arguments were passed in */

	RetVal = 0;

	/* Check to see if any multi arguments were actually passed requested */

	if (m_iMagicOption != -1)
	{
		/* Iterate through the multi argument ptrs and count how many are valid */

		if ((MultiArguments = (const char **) m_plArgs[m_iMagicOption]) != NULL)
		{
			while (*MultiArguments)
			{
				++RetVal;
				*MultiArguments++;
			}
		}
	}

	return(RetVal);
}

/* Written: Sunday 03-05-2010 9:38 am */

const char **RArgs::ExtractArguments(char *a_pcBuffer, TInt *a_piArgC)
{
	char *Arg;
	const char **RetVal;
	TInt Index, Offset, NumArgs;

	/* Find out how many arguments are present in the string passed in */

	// TODO: CAW - +1 bodginess
	NumArgs = (Utils::CountTokens(a_pcBuffer) + 1);

	/* Allocate an array of ptrs to strings, that can be used as an ArgV style array to hold ptrs */
	/* to the tokens entered above */

	if ((RetVal = new const char *[NumArgs]) != NULL)
	{
		TLex Lex(a_pcBuffer);

		/* Iterate through the command line entered and extract each token, NULL terminating it and */
		/* putting a ptr to it in the ArgV array */

		Offset = 0;

		for (Index = 1; Index < NumArgs; ++Index)
		{
			if ((Arg = Lex.NextToken()) != NULL)
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
		Utils::Info("RDArgs::ExtractArguments() => Out of memory");
	}

	return(RetVal);
}

/* Written: Thursday 16-Jul-2009 6:37 am */
/* @param	a_ppcOption	Ptr to a ptr into which to place the name of the option extracted. */
/*						May be NULL if you are only interested in the option's type */

TInt RArgs::ExtractOption(const char *a_pccTemplate, TInt *a_piOffset, char **a_ppcOption, char *a_pcType)
{
	char Char, *Option;
	TInt Offset, StartOffset, EndOffset, RetVal;

	/* Assume the option wasn't extracted successfully */

	RetVal = KErrNotFound;
	*a_pcType = '\0';

	if (a_ppcOption)
	{
		*a_ppcOption = NULL;
	}

	/* Iterate through the template, starting at the offset passed in, and look for the type separator for */
	/* the current option and the ',' that separates this option from the next one */

	Offset = StartOffset = EndOffset = *a_piOffset;

	for ( ; ; )
	{
		Char = a_pccTemplate[Offset];

		/* If we are at the end of the template or the current option then break out of the loop */

		if ((Char == '\0') || (Char == ','))
		{
			/* If no '/' was found then the option is optional but should still be extracted */

			if (EndOffset == StartOffset)
			{
				EndOffset = Offset;
			}

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

	/* If we have found a keyword terminated by a '/' then we have found an option so allocate */
	/* a buffer for it and extract the option so it can be returned.  In this case the type */
	/* of the option will already have been returned by the code above */

	if (StartOffset < EndOffset)
	{
		/* Indicate success */

		RetVal = KErrNone;

		/* Only return the option if the calling code has passed in a ptr into which to place this option */

		if (a_ppcOption)
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
	}

	return(RetVal);
}

/* Written: Saturday 29-Oct-2011 7:28 am, CodeHQ Söflingen */

void RArgs::FindMagicMultiOption(const char *a_pccTemplate, TInt a_iNumOptions)
{
	char Type;
	TInt Index, Offset, RetVal; // TODO: CAW

	Offset = 0;

	for (Index = 0; Index < a_iNumOptions; ++Index)
	{
		if ((RetVal = ExtractOption(a_pccTemplate, &Offset, NULL, &Type)) == KErrNone)
		{
			if (Type == 'M')
			{
				m_iMagicOption = Index;

				break;
			}
		}
	}
}

/* Written: Saturday 29-Oct-2011 7:54 am */

const char *RArgs::MultiArgument(TInt a_iIndex)
{
	const char *RetVal;
	const char **MultiArguments;

	RetVal = NULL;

	if (m_iMagicOption != -1)
	{
		MultiArguments = (const char **) m_plArgs[m_iMagicOption];

		if (MultiArguments)
		{
			RetVal = MultiArguments[a_iIndex];
		}
	}

	return(RetVal);
}

/* Written: Tuesday 13-Jan-2009 7:19 am */

const char *RArgs::ProjectFileName()
{
	return(m_pcProjectFileName);
}

#ifndef __amigaos4__

/* Written: Saturday 02-05-2010 8:52 am */

TInt RArgs::ReadArgs(const char *a_pccTemplate, TInt a_iNumOptions, const char *a_pccArgV[], TInt a_iArgC)
{
	char **ArgV, *OptionName, Type;
	TInt Arg, Index, Offset, RetVal;

	/* Assume success */

	RetVal = KErrNone;

	/* Allocate a buffer large enough to hold the arguments passed in from the command line.  This */
	/* is used to keep track of which arguments have been found already so that /A options can be */
	/* automatically filled in after scanning the other options */

	if ((ArgV = new char *[a_iArgC]) != NULL)
	{
		memcpy(ArgV, a_pccArgV, (sizeof(char *) * a_iArgC));
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
		/* options and optional options and using whatever arguments are leftover on the command line as their */
		/* values */

		if (RetVal == KErrNone)
		{
			Offset = 0;

			for (Index = 0; Index < a_iNumOptions; ++Index)
			{
				if ((RetVal = ExtractOption(a_pccTemplate, &Offset, &OptionName, &Type)) == KErrNone)
				{
					if ((Type == 'A') || (Type == '\0'))
					{
						/* Now scan through the arguments passed in looking for an unused one */

						for (Arg = 1; Arg < a_iArgC; ++Arg)
						{
							/* Only use the argument if it is not an empty string ("") */

							if ((ArgV[Arg]) && (strlen(ArgV[Arg]) > 0))
							{
								/* Found an unused argument so copy this into the args array so that it represents */
								/* the current A option when queried by client code.  Set the temporary copy to NULL */
								/* so that it is not reused again for the next A option */

								m_plArgs[Index] = (LONG) ArgV[Arg];
								ArgV[Arg] = NULL;

								break;
							}
						}

						/* If the option was an A option and it was not found then signal that an error has occurred */

						if ((Type == 'A') && (Arg == a_iArgC))
						{
							Utils::Info("RArgs::ReadArgs() => Option \"%s\" must have an argument", OptionName);

							RetVal = KErrNotFound;
							delete [] OptionName;

							break;
						}
					}

					// TODO: CAW - Ensure that this matches the quirky behaviour of the Amiga version as described
					//			   in the autodocs
					// TODO: CAW - This is pretty horrible as in the case of DEST/A/M, OptionName will be DEST/A
					//             and it's undescribed why DEST doesn't get handled by the above

					else if (Type == 'M')
					{
						/* Now scan through the arguments passed in looking for an unused one */

						for (Arg = 1; Arg < a_iArgC; ++Arg)
						{
							/* Only use the argument if it is not an empty string ("") */

							if ((ArgV[Arg]) && (strlen(ArgV[Arg]) > 0))
							{
								if (!(m_plArgs[m_iMagicOption]))
								{
									m_plArgs[m_iMagicOption] = (LONG) new char *[10]; // TODO: CAW
									char **Ptr = (char **) m_plArgs[m_iMagicOption];
									*Ptr = NULL;
								}

								if (m_plArgs[m_iMagicOption])
								{
									char **Ptr = (char **) m_plArgs[m_iMagicOption];

									while (*Ptr)
									{
										*Ptr++;
									}

									*Ptr = ArgV[Arg];
									*(Ptr + 1) = NULL;

									ArgV[Arg] = NULL;
								}
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

/* Written: Sunday 04-Nov-2007 11:57 am */

const char *RArgs::operator[](TInt a_iIndex)
{
	const char *RetVal;
	const char **MultiArguments;

	ASSERTM(((a_iIndex >= 0) && (a_iIndex < m_iNumArgs)), "RArgs::operator[]() => a_iIndex is out of range");

	if (a_iIndex == m_iMagicOption)
	{
		if ((MultiArguments = (const char **) m_plArgs[a_iIndex]) != NULL)
		{
			RetVal = *(const char **) m_plArgs[a_iIndex];
		}
		else
		{
			RetVal = NULL;
		}
	}
	else
	{
		RetVal = (const char *) m_plArgs[a_iIndex];
	}

	return(RetVal);
}
