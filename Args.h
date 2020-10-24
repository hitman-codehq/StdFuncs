
#ifndef ARGS_H
#define ARGS_H

/* Forward declaration to avoid including workbench/startup.h */

struct WBStartup;

/* This class allows easy parsing of both Workbench icons for tooltypes, and CLI command line */
/* parameters.  All that is needed is a command line template */

class RArgs
{
private:

	char			*m_pcArgumentBuffer;	/**< Buffer for emulating CLI command line when reading main() arguments */
	char			*m_pcCommandLine;		/**< Buffer for emulating CLI command line when running from Workbench */
	char			*m_pcProjectFileName;	/**< Name of project icon that was started with the program, if any */
	TInt			m_iMagicOption;			/**< Index of magic option, if any */
	TInt			m_iNumMagicArgs;		/**< Number of entries in the magic option array */
	TInt			m_iNumArgs;				/**< Number of entries in the iArgs array */
	size_t			*m_pstArgs;				/**< Array of size_ts into which to place pointers to arguments */
	struct RDArgs	*m_poRDArgs;			/**< Structure for use by ReadArgs() when reading arguments */
	struct RDArgs	*m_poTTRDArgs;			/**< Structure for use by ReadArgs() when reading tooltypes */
	struct RDArgs	*m_poInputRDArgs;		/**< Structure for use by ReadArgs() when reading main() arguments */
	std::string		m_oArguments;			/**< String into which arguments are read from stdin when ? argument is used.
												 These are referenced by the m_pstArgs array and must be persistent */

private:

	const char **ExtractArguments(char *a_pcBuffer, TInt *a_piArgC);

	TInt ExtractOption(const char *a_pccTemplate, TInt *a_piOffset, char **a_ppcOption, char *a_pcType);

	void FindMagicOption(const char *a_pccTemplate, TInt a_iNumOptions);

	TInt readArgs(const char *a_pccTemplate, TInt a_iNumOptions, const char *a_pccArgV[], TInt a_iArgC);

public:

	RArgs();

	TInt open(const char *a_pccTemplate, TInt a_iNumOptions, const char *a_pccArgV[], TInt a_iArgC);

	TInt open(const char *a_pccTemplate, TInt a_iNumOptions, char *a_pcArguments);

	TInt open(const char *a_pccTemplate, TInt a_iNumOptions, const struct WBStartup *a_poWBStartup);

	void close();

	TInt Count();

	TInt CountMultiArguments();

	const char *MultiArgument(TInt a_iIndex);

	const char *ProjectFileName();

	const char *operator[](TInt a_iIndex);
};

#endif /* ! ARGS_H */
