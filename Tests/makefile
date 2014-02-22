
CC = @g++
LD = @g++
CFLAGS = -c -Wall -Wextra -Wwrite-strings
IFLAGS = -I../
LIBS = -lStdFuncs

ifdef DEBUG
	OBJ = Debug
	CFLAGS += -ggdb -D_DEBUG
else
	OBJ = Release
	CFLAGS += -O2
endif

UNAME = $(shell uname)

ifeq ($(UNAME), AmigaOS)

LIBS += -lauto

endif

LFLAGS += -L../$(OBJ)

EXECUTABLES = $(T_ARGS) $(T_CONFIGFILE) $(T_DIR) $(T_FILE) $(T_LEX) $(T_POOL) $(T_STRINGLIST) $(T_UTILS) $(T_WILDCARD)

T_ARGS = $(OBJ)/T_Args

T_CONFIGFILE = $(OBJ)/T_ConfigFile

T_DIR = $(OBJ)/T_Dir

T_FILE = $(OBJ)/T_File

T_LEX = $(OBJ)/T_Lex

T_POOL = $(OBJ)/T_Pool

T_STRINGLIST = $(OBJ)/T_StringList

T_UTILS = $(OBJ)/T_Utils

T_WILDCARD = $(OBJ)/T_Wildcard

T_ARGS_OBJECTS = $(OBJ)/T_Args.o

T_CONFIGFILE_OBJECTS = $(OBJ)/T_ConfigFile.o

T_DIR_OBJECTS = $(OBJ)/T_Dir.o

T_FILE_OBJECTS = $(OBJ)/T_File.o

T_LEX_OBJECTS = $(OBJ)/T_Lex.o

T_POOL_OBJECTS = $(OBJ)/T_Pool.o

T_STRINGLIST_OBJECTS = $(OBJ)/T_StringList.o

T_UTILS_OBJECTS = $(OBJ)/T_Utils.o

T_WILDCARD_OBJECTS = $(OBJ)/T_Wildcard.o

All: $(OBJ) $(EXECUTABLES)

$(OBJ):
	@mkdir $(OBJ)

$(T_ARGS): $(T_ARGS_OBJECTS) ../$(OBJ)/libStdFuncs.a
	@echo Linking $@...
	$(LD) $(T_ARGS_OBJECTS) $(LFLAGS) $(LIBS) -o $@.debug
	@strip -R.comment $@.debug -o $@

$(T_CONFIGFILE): $(T_CONFIGFILE_OBJECTS) ../$(OBJ)/libStdFuncs.a
	@echo Linking $@...
	$(LD) $(T_CONFIGFILE_OBJECTS) $(LFLAGS) $(LIBS) -o $@.debug
	@strip -R.comment $@.debug -o $@

$(T_DIR): $(T_DIR_OBJECTS) ../$(OBJ)/libStdFuncs.a
	@echo Linking $@...
	$(LD) $(T_DIR_OBJECTS) $(LFLAGS) $(LIBS) -o $@.debug
	@strip -R.comment $@.debug -o $@

$(T_FILE): $(T_FILE_OBJECTS) ../$(OBJ)/libStdFuncs.a
	@echo Linking $@...
	$(LD) $(T_FILE_OBJECTS) $(LFLAGS) $(LIBS) -o $@.debug
	@strip -R.comment $@.debug -o $@

$(T_LEX): $(T_LEX_OBJECTS) ../$(OBJ)/libStdFuncs.a
	@echo Linking $@...
	$(LD) $(T_LEX_OBJECTS) $(LFLAGS) $(LIBS) -o $@.debug
	@strip -R.comment $@.debug -o $@

$(T_POOL): $(T_POOL_OBJECTS) ../$(OBJ)/libStdFuncs.a
	@echo Linking $@...
	$(LD) $(T_POOL_OBJECTS) $(LFLAGS) $(LIBS) -o $@.debug
	@strip -R.comment $@.debug -o $@

$(T_STRINGLIST): $(T_STRINGLIST_OBJECTS) ../$(OBJ)/libStdFuncs.a
	@echo Linking $@...
	$(LD) $(T_STRINGLIST_OBJECTS) $(LFLAGS) $(LIBS) -o $@.debug
	@strip -R.comment $@.debug -o $@

$(T_UTILS): $(T_UTILS_OBJECTS) ../$(OBJ)/libStdFuncs.a
	@echo Linking $@...
	$(LD) $(T_UTILS_OBJECTS) $(LFLAGS) $(LIBS) -o $@.debug
	@strip -R.comment $@.debug -o $@

$(T_WILDCARD): $(T_WILDCARD_OBJECTS) ../$(OBJ)/libStdFuncs.a
	@echo Linking $@...
	$(LD) $(T_WILDCARD_OBJECTS) $(LFLAGS) $(LIBS) -o $@.debug
	@strip -R.comment $@.debug -o $@

$(OBJ)/%.o: %.cpp
	@echo Compiling $<...
	$(CC) $(CFLAGS) $(IFLAGS) -o $(OBJ)/$*.o $<

clean:
	@rm -fr $(OBJ)
