
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
	m_pcArgumentBuffer = m_pcCommandLine = m_pcProjectFileName = NULL;
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

	/* Allocate an array of LONGs into which ptrs to arguments can be placed */

	if ((m_plArgs = new LONG[m_iNumArgs]) != NULL)
	{
		/* Determine which if option (if any) is the magic multi option */

		FindMagicOption(a_pccTemplate, a_iNumOptions);

		/* Now read the arguments, according to the argument template */

#ifdef __amigaos4__

		int Index, Size;

		(void) a_pccArgV;
		(void) a_iArgC;

		/* Assume failure */

		RetVal = KErrNoMemory;

		/* For Amiga OS we can let dos.library do the hard work for us.  However, for */
		/* compatibility with other operating systems we use the arguments passed in, */
		/* rather than the internal ones originally parsed by dos.library */

		m_poInputRDArgs	= (struct RDArgs *) IDOS->AllocDosObjectTags(DOS_RDARGS, TAG_DONE);

		if (m_poInputRDArgs)
		{
			/* Start with a size of 2 to hold the '\n' EOL character and NULL terminator */

			Size = 2;

			/* Determine how much memory to allocate to hold all of the arguments.  We */
			/* allocate extra memory for a space between each argument and also for " */
			/* delimiters, which we must use or special Amiga characters such as '=" in */
			/* filenames will not work */

			for (Index = 1; Index < a_iArgC; ++Index)
			{
				Size += (strlen(a_pccArgV[Index]) + 3);
			}

			/* Allocate the buffer and copy the arguments into it */

			if ((m_pcArgumentBuffer = new char[Size]) != NULL)
			{
				m_pcArgumentBuffer[0] = '\0';

				for (Index = 1; Index < a_iArgC; ++Index)
				{
					if (strstr(a_pccArgV[Index], " ") > 0) strcat(m_pcArgumentBuffer, "\"");
					strcat(m_pcArgumentBuffer, a_pccArgV[Index]);
					if (strstr(a_pccArgV[Index], " ") > 0) strcat(m_pcArgumentBuffer, "\"");

					/* If this is not the last argument, append a space to it */

					if ((Index + 1) < a_iArgC)
					{
						strcat(m_pcArgumentBuffer, " ");
					}
				}

				strcat(m_pcArgumentBuffer, "\n");

				/* Put the arguments into the RDArgs structure for parsing */

				m_poInputRDArgs->RDA_Source.CS_Buffer = m_pcArgumentBuffer;
				m_poInputRDArgs->RDA_Source.CS_Length = Size;

				/* And try and parse them! */

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
						RetVal = KErrNoMemory;
					}

					Utils::Info("RArgs::Open() => Unable to read command line arguments");
				}
			}
			else
			{
				Utils::Info("RArgs::Open() => Out of memory");
			}
		}
		else
		{
			Utils::Info("RArgs::Open() => Unable to allocate RDArgs structure");
		}

#else /* ! __amigaos4__ */

		const char **ArgV, *BufferPtr;
		TInt ArgC;

		/* If the user has requested to display the template then do so and request input and parse that instead */
		/* of what was passed in on the command line */

		if ((a_iArgC == 2) && (*a_pccArgV[1] == '?'))
		{
			/* Display the template à la Amiga OS and prompt the user for input */

			// TODO: CAW - This doesn't work with WinMain() style programs so NULL termination below is temporary + input buffer?
			char Buffer[1024];

			Buffer[0] = '\0';
			printf("%s\n", a_pccTemplate);

			/* Obtain the input from the user */

			if ((BufferPtr = gets(Buffer)) != NULL)
			{
				/* Extract the arguments from the buffer into an ArgV style ptr array.  This */
				/* will result in ArgC being the number of arguments + 1 as ExtractArguments() */
				/* adds an extra entry for the executable name as the first argument.  This */
				/* is required so that when we call Open() it will ignore this first argument, */
				/* for compatibility with argument lists passed into main(), which also have */
				/* the executable name in argv[0]! */

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
				RetVal = KErrGeneral;
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

/* Written: Friday 04-Jun-2010 8:36 am */
/* @param	a_pcArguments	Ptr to a string containing a list of arguments passed in */
/*							from the command line.  Note that this is a Windows style list */
/*							as passed into WinMain() and thus the first argument is NOT */
/*							the executable name */

TInt RArgs::Open(const char *a_pccTemplate, TInt a_iNumOptions, char *a_pcArguments)
{
	const char **ArgV;
	TInt ArgC, RetVal;

	/* Extract the arguments from the string into an ArgV style ptr array.  This */
	/* will result in ArgC being the number of arguments + 1 as ExtractArguments() */
	/* adds an extra entry for the executable name as the first argument.  This */
	/* is required so that when we call Open() it will ignore this first argument, */
	/* for compatibility with argument lists passed into main(), which also have */
	/* the executable name in argv[0]! */

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

#ifdef __amigaos4__

TInt RArgs::Open(const char *a_pccTemplate, TInt a_iNumOptions, const struct WBStartup *a_poWBStartup)
{
	TInt RetVal;

	char *Source, *Dest;
	TBool Directory, FoundEquals;
	TInt Index, Length;
	STRPTR *ToolTypes;
	struct DiskObject *DiskObject;
	struct RDArgs *RDArgs;
	struct WBArg *WBArg;

	/* Assume failure */

	RetVal = KErrNoMemory;

	/* Allocate an array of LONGs into which ptrs to arguments can be placed */

	if ((m_plArgs = new LONG[a_iNumOptions]) != NULL)
	{
		/* Save the # of arguments for l8r */

		m_iNumArgs = a_iNumOptions;

		/* Get some information about the .info file associated with the WBStartup message passed in */

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

#endif /* __amigaos4__ */

/* Written: Sunday 04-Nov-2007 11:52 am */

void RArgs::Close()
{
	/* Delete the emulated command line arguments buffer */

	delete [] m_pcCommandLine;
	m_pcCommandLine = NULL;

	/* Delete the buffer used for holding the project filename */

	delete [] m_pcProjectFileName;
	m_pcProjectFileName = NULL;

#ifndef __amigaos4__

	/* Delete the array used for the magic multi args */

	if (m_iMagicOption != -1)
	{
		/* Unlike the other options, which are ptrs to unowned memory, the magic option */
		/* points to an array of ptrs to arguments and this array is owned by the magic */
		/* option itself.  Thus we must cast the data for the option in order to delete it */

		delete [] (char **) (m_plArgs[m_iMagicOption]);
		m_iMagicOption = -1;
	}

#endif /* ! __amigaos4__ */

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

	/* Free the argument structures, if they exist */

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

	/* And free the buffer used for holding copies of the arguments */

	delete m_pcArgumentBuffer;
	m_pcArgumentBuffer = NULL;

#endif /* __amigaos4__ */

	/* Ensure that everything is back to the exact state it was in before Open() was called */

	m_iNumArgs = 0;
	m_iMagicOption = -1;
}

/* Written: Sunday 04-Nov-2007 12:12 pm */

TInt RArgs::Count()
{
	return(m_iNumArgs);
}

/* Written: Saturday 29-Oct-2011 8:05 am, Code HQ Söflingen */

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
				++MultiArguments;
			}
		}
	}

	return(RetVal);
}

/* Written: Sunday 03-05-2010 9:38 am */
/* @param	a_pcBuffer	Ptr to buffer to be parsed */
/*			a_piArgC	Ptr to variable into which to place argument count */
/* @return	Ptr to an array of ptrs to the arguments extracted */
/* This function will parse a string that contains a list of white space separated */
/* arguments, and will extract each argument into a separate string buffer.  It will */
/* then allocate a list of ptrs to these buffers which simulates a main() style ArgV */
/* list of ptrs.  This list will include an empty (points to NULL) entry in position 0 */
/* to simulate the name of the executable being in this entry */

const char **RArgs::ExtractArguments(char *a_pcBuffer, TInt *a_piArgC)
{
	char *Arg;
	const char **RetVal;
	TInt Index, Offset, NumArgs;

	/* Find out how many arguments are present in the string passed in and add */
	/* one for the executable name entry */

	NumArgs = (Utils::CountTokens(a_pcBuffer) + 1);

	/* Allocate an array of ptrs to strings, that can be used as an ArgV style array to hold ptrs */
	/* to the tokens entered above */

	if ((RetVal = new const char *[NumArgs]) != NULL)
	{
		TLex Lex(a_pcBuffer);

		/* Make the executable name entry "empty" */

		RetVal[0] = NULL;

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
/* @param	a_pccTemplate		Ptr to Amiga OS style template to be parsed */
/*			a_piOffset			Ptr to variable containing offset to which template has been parsed */
/*			a_ppcOption			Ptr to a ptr into which to place a ptr to the name of the option extracted. */
/*								May be NULL if you are only interested in the option's type */
/*			a_pcType			Ptr to a variable into which to place the type of the option, such */
/*								as 'M' or 'A' etc. */
/* @return	KErrNone if the option and its type were extracted successfully */
/*			KErrNotFound if no more options are present in the template passed in */
/*			KErrNoMemory if not enough memory was available */
/* Parses an Amiga OS style option string, starting at the offset passed in, and extracts the next option */
/* option available at that offset, as well as its type, such as 'M' or 'A' */

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
			/* Special handling for the magic option, as this overrides any other types of options */
			/* and we don't care if a magic option is also an /A option etc. */

			if (*a_pcType != 'M')
			{
				EndOffset = Offset;
				*a_pcType = a_pccTemplate[Offset + 1];
			}
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

/* Written: Saturday 29-Oct-2011 7:28 am, Code HQ Söflingen */

void RArgs::FindMagicOption(const char *a_pccTemplate, TInt a_iNumOptions)
{
	char Type;
	TInt Index, Offset, RetVal;

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
	char **ArgV, *OptionName, **Arguments, Type;
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
						if ((ArgV[Arg]) && (!(_stricmp(ArgV[Arg], OptionName))))
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
		/* options and optional options, using whatever arguments are leftover on the command line as their */
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

						/* If the option was an A option and it was not found then try to steal an argument */
						/* from the multi argument list */

						if ((Type == 'A') && (Arg == a_iArgC))
						{
							/* Assume failure */

							RetVal = KErrNotFound;

							/* If there is a multi argument list present then try and steal an argument */

							if ((m_iMagicOption != -1) && (m_plArgs[m_iMagicOption]))
							{
								/* Scan through the multi argument list until we either find an argument or */
								/* reach the end of the list */

								Arguments = (char **) m_plArgs[m_iMagicOption];

								while (*Arguments)
								{
									if (*(Arguments + 1))
									{
										++Arguments;
									}
									else
									{
										break;
									}
								}

								/* If we have found an argument then "steal" it by moving it into the current */
								/* option's slot.  This is consistent with the behaviour of Amiga OS */

								if (*Arguments)
								{
									RetVal = KErrNone;
									m_plArgs[Index] = (LONG) *Arguments;
									*Arguments = NULL;
								}
							}

							if (m_plArgs[Index] == 0)
							{
								Utils::Info("RArgs::ReadArgs() => Option \"%s\" must have an argument", OptionName);
							}

							delete [] OptionName;

							break;
						}
					}
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
										++Ptr;
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
/* @param	a_iIndex	Index of the argument to retrieve */
/* @return	Ptr to the argument passed in, or NULL if no argument was passed in */
/* This function will return the argument that was passed in that matches the option */
/* in the template used for parsing arguments.  The index of the argument matches the */
/* index of the option in the template.  Thus if COPY/A was the first option then its */
/* argument will be at index 0.  If DELETE/A was the second option then its argument */
/* will be at index 1 etc. */

const char *RArgs::operator[](TInt a_iIndex)
{
	const char *RetVal;

	ASSERTM(((a_iIndex >= 0) && (a_iIndex < m_iNumArgs)), "RArgs::operator[]() => a_iIndex is out of range");

	/* If the magic option is being requested then we need to handle it specially as the */
	/* argument indexed will not point to the argument itself, but to an array of ptrs to */
	/* arguments assigned to the magic option */

	if (a_iIndex == m_iMagicOption)
	{
		/* If the magic option was filled in then extract the first argument that was */
		/* assigned to the option */

		if (m_plArgs[m_iMagicOption])
		{
			RetVal = *(const char **) m_plArgs[m_iMagicOption];
		}
		else
		{
			RetVal = NULL;
		}
	}

	/* Otherwise this is a normal option so just return whatever argument was assigned to it */

	else
	{
		RetVal = (const char *) m_plArgs[a_iIndex];
	}

	return(RetVal);
}
