
#ifndef STDCONFIGFILE_H
#define STDCONFIGFILE_H

/* This class will allow the client to easily parse Windows style .ini files, */
/* querying the class individual entries of a known name and also enumerating */
/* entries for which only a part of the name is known */

class RConfigFile
{
	char	*m_pcBuffer;	/* Ptr to buffer containing Windows style .ini file */
	TInt	m_iBufferSize;	/* # of bytes in the file, including NULL terminator */

public:

	RConfigFile()
	{
		m_pcBuffer = NULL;
	}

	~RConfigFile()
	{
		Close();
	}

	TInt Open(const char *a_pccFileName);

	void Close();

	TInt GetInteger(const char *a_pccSectionName, const char *a_pccSubSectionName,
		const char *a_pccKeyName, TInt *a_piResult);

	void GetString(const char *a_pccSectionName, const char *a_pccSubSectionName,
		const char *a_pccKeyName, char *&a_rpcResult);
};

#endif /* ! STDCONFIGFILE_H */
