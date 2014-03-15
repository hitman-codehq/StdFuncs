
#ifndef STDSTRINGLIST_H
#define STDSTRINGLIST_H

/**
 * A class to easily split strings into individual words.
 * This class allows splitting strings into individual words in a style similar to that used
 * by the Python string.split() function.  Like its Python equivalent, it endeavours to make
 * this task as simple as possible and to hide away all of the tedious details of the task, such
 * as the management of the memory used to hold the strings and parsing the input string for
 * separators and white space.
 */

class CStdStringList
{
private:

	char	**m_ppcStrings;				/**< Array of ptrs to keywords to be highlighted */
	TInt	m_iNumStrings;				/**< Number of keywords present in the m_ppcKeywords array */

public:

	/**
	 * Destructor for the CStdStringList class.
	 * Frees up the strings owned by the class, as well as the array used to keep track
	 * of their ptrs.
	 *
	 * @date	Tuesday 19-Nov-2013 7:56 am, Code HQ Ehinger Tor
	 */

	~CStdStringList()
	{
		Reset();
	}

	TInt Append(const char *a_pccString);

	/**
	 * Retrieves the number of strings contained in the array.
	 * Returns the number of strings that have been parsed into the array.  The strings may be
	 * accessed via the [] operator using and index between 0 and the value returned by this
	 * function.
	 *
	 * @date	Tuesday 19-Nov-2013 7:54 am, Code HQ Ehinger Tor
	 * @return	The number of strings contained in the array
	 */

	TInt Count()
	{
		return(m_iNumStrings);
	}

	void Reset();

	const char *operator[](TInt a_iIndex);
};

#endif /* ! STDSTRINGLIST_H */
