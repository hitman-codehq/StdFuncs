
#ifndef STDCONFIGFILE_H
#define STDCONFIGFILE_H

#include "File.h"

class RConfigFile
{
  char *pBuffer;
  TInt iBufferSize;
  RFile fConfigFile;     /* File class used for config file I/O */
  BOOL bConfigFileOpen; /* TRUE if the config file is open, else FALSE */

public:

  RConfigFile() { pBuffer = NULL; bConfigFileOpen = FALSE; }
  ~RConfigFile() { CloseConfigFile(); }
  TInt OpenConfigFile(const char *);
  void CloseConfigFile(void);
  void GetConfigInteger(const char *, const char *, const char *, int *);
  void GetConfigString(const char *, const char *, const char *, char *);
};

#endif /* ! STDCONFIGFILE_H */
