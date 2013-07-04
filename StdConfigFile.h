
#ifndef STDCONFIGFILE_H
#define STDCONFIGFILE_H

/**
 * Represents a single key::value pair from a configuration file.
 * This class represents a key and its value that has been found inside a section.  It is
 * not meant to be created by client code (although there is nothing to prevent you from
 * using it if you find a useful purpose for it) but is created and returned when using the
 * configuration file parsing class RConfigFile and its associated CSection class.  When
 * creating configuration files, keys and their values can be placed inside a subsection or
 * a group but not a top level section.
 */

class CKey
{
public:

	StdListNode<CKey>		m_oStdListNode;	/**< Standard list node */
	char					*m_pcName;		/**< Name of the key represented by this node */
	char					*m_pcValue;		/**< Value of the key represented by this node */

public:

	static CKey *New(const char *a_pccKey, TInt a_iKeyLength, const char *a_pccValue, TInt a_iValueLength);

	~CKey()
	{
		delete [] m_pcName;
		delete [] m_pcValue;
	}
};

/**
 * Represents a single section, subsection or group in a configuration file.
 * This class represents a section, subsection or group that has been found inside a section.
 * It is not meant to be created by client code (although there is nothing to prevent you from
 * using it if you find a useful purpose for it) but is created and returned when using the
 * configuration file parsing class RConfigFile and its associated CSection class.  A
 * configuration file can contain sections, marked as (SectionName), subsections, marked as
 * [SubsectionName], and groups, marked as {GroupName}.  When using RConfigFile to parse a
 * configuration file, the sections, subsections and groups found will be placed into a list
 * of instances of this CSection class, and client code can query the RConfigFile instance
 * about them.  Once the desired section has been found, the section itself can be queried
 * about which key it contains.  Theys keys are represented by instances of the CKey class.
 */

class CSection
{
public:

	StdListNode<CSection>	m_oStdListNode;	/**< Standard list node */
	StdList<CSection>		m_oSections;	/**< List of subsections belonging to this section */
	StdList<CKey>			m_oKeys;		/**< List of keys and values belonging to this section */
	char					*m_pcName;		/**< Name of the section */

public:

	static CSection *New(const char *a_pccName, TInt a_iLength);

	~CSection();

	CSection *FindSection();

	CSection *FindSection(const char *a_pccName);

	CSection *FindNextSection(CSection *a_poSection);

	CKey *FindKey(const char *a_pccName);

	CKey *FindNextKey(CKey *a_poKey, const char *a_pccName);
};

/**
 * Class for scanning Windows style .ini configuration files.
 * This class will allow the client to easily parse Windows style .ini files,
 * extracting information about the sections, subsections, groups, keys and values
 * that are prsent therein.  Configuration files have the following format:
 *
 * (SectionName)
 *
 * [SubsectionName]\n
 * Key = Value
 *
 * {GroupName}\n
 * Key = Value
 *
 * The SectionName is required and identifies the application to which the file belongs.
 * It is legal to have multiple sections in one file if the client code has a reason
 * for it.  The SubsectionName represents a logical set of configuration values that
 * the application needs.  For example, an editor might have a [General] subsection for
 * general preferences, and a [SyntaxHighlighting] subsection for preferences related
 * to syntax highlighting.  Finally the GroupName represents a subsection of the
 * subsection.  Both subsections and groups can be enumerated, so they can be used for
 * storing groupings of arbitrary data.  Again, using the text editor example, inside
 * the [Syntaxhighlighting] subsection might be a number of groups representing the
 * filetypes represented by the editor, such as {C/C++}, {Python} etc.  Inside these
 * groups are the Key = Value pairs that represent such attributes of the file as
 * keywords, comment characters, colours etc.  The editor is able to store an arbitrary
 * number of filetypes inside a single subsection and can access their Key::Value pairs
 * knowing only the name of the keys required and the name of the subsection in which
 * the groups reside, namely [SyntaxHighlighting].
 *
 * Once the file has been opened and parsed, there are two ways to query the value
 * of the desired keys.  Firstly is the direct query method.  This is a simplified
 * method whereby you need to know in advance not only the name of the key you
 * require, but the section, subsection and possibly group in which it exists.  This
 * API is used via the functions GetInteger() and GetString().
 *
 * Secondly is the enumeration query method.  This is the method to use where you
 * do not know which subsections and groups are present and need to iterate through
 * them until you find ones you are interested in.  This API is used via the function
 * FindSection().  You use this to return a CSection of the desired name (known top
 * level sections are a requirement of the configuration file format - you cannot
 * enumerate them) and once this is aquired, you call the various Find*() and
 * FindNext*() functions in CSection to iterate through the subsections and groups
 * present until you have found the one you are interested in.  At this point you
 * may query the subsection or group for the desired key::value pairs.
 */

class RConfigFile
{
private:

	char				*m_pcBuffer;	/**< Ptr to buffer containing Windows style .ini file */
	TInt				m_iBufferSize;	/**< # of bytes in the file */
	StdList<CSection>	m_oSections;	/**< List of the top level sections in the configuration file */

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
