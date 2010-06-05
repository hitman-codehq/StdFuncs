
#ifndef ARGS_H
#define ARGS_H

/* Forward declaration to void including workbench/startup.h */

struct WBStartup;

/* This class allows easy parsing of both Workbench icons for tooltypes, and CLI command line */
/* parameters.  All that is needed is a command line template */

class RArgs
{
public:

	RArgs();

	TInt Open(const char *a_pccTemplate, TInt a_iNumOptions, const char *a_pccArgV[], TInt a_iArgC);

	TInt Open(const char *a_pccTemplate, TInt a_iNumOptions, char *a_pcArguments);

	TInt Open(const char *a_pccTemplate, TInt a_iNumOptions, const struct WBStartup *a_poWBStartup);

	void Close();

	TInt Count();

	const char *ProjectFileName();

	TInt Valid();

	const char *operator[](TInt a_iIndex);

private:

	const char **ExtractArguments(char *a_pcBuffer, TInt *a_piArgC);

	TInt ExtractOption(const char *a_pccTemplate, TInt *a_piOffset, char **a_ppcOption, char *a_pcType);

	TInt ReadArgs(const char *a_pccTemplate, TInt a_iNumOptions, const char *a_pccArgV[], TInt a_iArgC);

private:

	char			*m_pcCommandLine;		/* Buffer for emulating CLI command line when running from Workbench */
	char			*m_pcProjectFileName;	/* Name of project icon that was started with the program, if any */
	TInt			m_iNumArgs;				/* # of entries in the iArgs array */
	LONG			*m_plArgs;				/* Array of LONGs into which to place ptrs to arguments */
	struct RDArgs	*m_poRDArgs;			/* Structure for use by IDOS->ReadArgs() when reading arguments */
	struct RDArgs	*m_poTTRDArgs;			/* Structure for use by IDOS->ReadArgs() when reading tooltypes */
};

#endif /* ! ARGS_H */
