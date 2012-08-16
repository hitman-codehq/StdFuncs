
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

LFLAGS += -lauto

endif

LFLAGS += -L../$(OBJ)

EXECUTABLES = $(T_ARGS) $(T_DIR) $(T_FILE) $(T_UTILS) $(T_WILDCARD)

T_ARGS = $(OBJ)/T_Args

T_DIR = $(OBJ)/T_Dir

T_FILE = $(OBJ)/T_File

T_UTILS = $(OBJ)/T_Utils

T_WILDCARD = $(OBJ)/T_Wildcard

T_ARGS_OBJECTS = $(OBJ)/T_Args.o

T_DIR_OBJECTS = $(OBJ)/T_Dir.o

T_FILE_OBJECTS = $(OBJ)/T_File.o

T_UTILS_OBJECTS = $(OBJ)/T_Utils.o

T_WILDCARD_OBJECTS = $(OBJ)/T_Wildcard.o

All: $(OBJ) $(EXECUTABLES)

$(OBJ):
	@mkdir $(OBJ)

$(T_ARGS): $(T_ARGS_OBJECTS) ../$(OBJ)/libStdFuncs.a
	@echo Linking $@...
	$(LD) $(T_ARGS_OBJECTS) $(LFLAGS) $(LIBS) -o $@.debug
	@strip -R.comment $@.debug -o $@

$(T_DIR): $(T_DIR_OBJECTS) ../$(OBJ)/libStdFuncs.a
	@echo Linking $@...
	$(LD) $(T_DIR_OBJECTS) $(LFLAGS) $(LIBS) -o $@.debug
	@strip -R.comment $@.debug -o $@

$(T_FILE): $(T_FILE_OBJECTS) ../$(OBJ)/libStdFuncs.a
	@echo Linking $@...
	$(LD) $(T_FILE_OBJECTS) $(LFLAGS) $(LIBS) -o $@.debug
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
