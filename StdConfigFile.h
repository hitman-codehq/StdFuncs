
#ifndef STDCONFIGFILE_H
#define STDCONFIGFILE_H

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

	TInt Open(const char *a_pcFileName);

	void Close();

	void GetInteger(const char *a_pccSectionName, const char *a_pccSubSectionName,
		const char *a_pccKeyName, int *a_piResult);

	void GetString(const char *a_pccSectionName, const char *a_pccSubSectionName,
		const char *a_pccKeyName, char *&a_rpcResult);
};

#endif /* ! STDCONFIGFILE_H */
