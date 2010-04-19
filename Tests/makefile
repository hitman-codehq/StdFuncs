
CC = @g++
LD = @g++
CFLAGS = -c -Wall -Wextra -Wwrite-strings
IFLAGS = -I/StdFuncs
LFLAGS = -lStdFuncs -lauto

ifdef DEBUG
	OBJ = Debug
	CFLAGS += -ggdb -D_DEBUG
else
	OBJ = Release
	CFLAGS += -O2
endif

LFLAGS += -L/StdFuncs/$(OBJ)

EXECUTABLES = $(T_ARGS) $(T_DIR) $(T_FILE)

T_ARGS = $(OBJ)/T_Args

T_DIR = $(OBJ)/T_Dir

T_FILE = $(OBJ)/T_File

T_ARGS_OBJECTS = $(OBJ)/T_Args.o

T_DIR_OBJECTS = $(OBJ)/T_Dir.o

T_FILE_OBJECTS = $(OBJ)/T_File.o

All: $(OBJ) $(EXECUTABLES)

$(OBJ):
	@MakeDir $(OBJ)

$(T_ARGS): $(T_ARGS_OBJECTS)
	@echo Linking $@...
	$(LD) $(T_ARGS_OBJECTS) $(LFLAGS) -o $@.debug
	@strip -R.comment $@.debug -o $@

$(T_DIR): $(T_DIR_OBJECTS)
	@echo Linking $@...
	$(LD) $(T_DIR_OBJECTS) $(LFLAGS) -o $@.debug
	@strip -R.comment $@.debug -o $@

$(T_FILE): $(T_FILE_OBJECTS)
	@echo Linking $@...
	$(LD) $(T_FILE_OBJECTS) $(LFLAGS) -o $@.debug
	@strip -R.comment $@.debug -o $@

$(OBJ)/%.o: %.cpp
	@echo Compiling $<...
	$(CC) $(CFLAGS) $(IFLAGS) -o $(OBJ)/$*.o $<

clean:
	@Delete $(OBJ) all quiet
