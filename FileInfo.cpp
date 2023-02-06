
#include "StdFuncs.h"
#include "FileInfo.h"

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Saturday 25-Feb-2023 6:06 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

int RFileInfo::open(const char *a_fileName)
{
    return Utils::GetFileInfo(a_fileName, &m_fileInfo);
}
