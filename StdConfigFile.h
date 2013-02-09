
#ifndef STDCONFIGFILE_H
#define STDCONFIGFILE_H

/* The CKey class represents a key and its value that has been found inside a section. */
/* Key and values can be placed inside a subsection or a group but not a top level section. */

class CKey
{
public:

	StdListNode<CKey>		m_oStdListNode;	/* Standard list node */
	char					*m_pcName;		/* Name of the key represented by this node */
	char					*m_pcValue;		/* Value of the key represented by this node */

public:

	static CKey *New(const char *a_pccKey, TInt a_iKeyLength, const char *a_pccValue, TInt a_iValueLength);

	~CKey()
	{
		delete [] m_pcName;
		delete [] m_pcValue;
	}
};

/* The CSection class represents a section from a configuration file.  Sections may be embedded */
/* within one another in the format Section::Subsection::Group and all of these are represented */
/* by this one class.  With it you can query for particular subsections or keys present within */
/* the section */

class CSection
{
public:

	StdListNode<CSection>	m_oStdListNode;	/* Standard list node */
	StdList<CSection>		m_oSections;	/* List of subsections belonging to this section */
	StdList<CKey>			m_oKeys;		/* List of keys and values belonging to this section */
	char					*m_pcName;		/* Name of the section */

public:

	static CSection *New(const char *a_pccName, TInt a_iLength);

	~CSection();

	CSection *FindSection();

	CSection *FindSection(const char *a_pccName);

	CSection *FindNextSection(CSection *a_poSection);

	CKey *FindKey(const char *a_pccName);

	CKey *FindNextKey(CKey *a_poKey, const char *a_pccName);
};

/* This class will allow the client to easily parse Windows style .ini files, */
/* querying the class individual entries of a known name and also enumerating */
/* entries for which only a part of the name is known */

class RConfigFile
{
	char				*m_pcBuffer;	/* Ptr to buffer containing Windows style .ini file */
	TInt				m_iBufferSize;	/* # of bytes in the file, including NULL terminator */
	StdList<CSection>	m_oSections;	/* List of the top level (Sections) in the configuration file */

private:

	TInt Parse();

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

	CSection *FindSection(const char *a_pccName);

	TInt GetInteger(const char *a_pccSectionName, const char *a_pccSubSectionName,
		const char *a_pccKeyName, TInt *a_piResult);

	TInt GetString(const char *a_pccSectionName, const char *a_pccSubSectionName,
		const char *a_pccKeyName, char *&a_rpcResult);
};

#endif /* ! STDCONFIGFILE_H */
