
#ifndef WILDCARD_H
#define WILDCARD_H

class RWildcard
{
private:

	const char	*m_pccWildcard;	/**< Ptr to raw wildcard to match */

public:

	TInt Open(const char *a_pccWildcard);

	void Close();

	TBool Match(const char *a_pccFileName);
};

#endif /* ! WILDCARD_H */
