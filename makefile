
AR = @ar
CC = @g++
CFLAGS = -c -fno-strict-aliasing -Wall -Wextra -Wwrite-strings

ifdef DEBUG
	OBJ = Debug
	CFLAGS += -ggdb -D_DEBUG
else
	OBJ = Release
	CFLAGS += -O2
endif

LIBRARY = $(OBJ)/libStdFuncs.a

OBJECTS = $(OBJ)/Args.o $(OBJ)/BaUtils.o $(OBJ)/Dir.o $(OBJ)/File.o $(OBJ)/Lex.o $(OBJ)/MungWall.o \
	$(OBJ)/StdApplication.o $(OBJ)/StdClipboard.o $(OBJ)/StdDialog.o $(OBJ)/StdFileRequester.o $(OBJ)/StdFont.o \
	$(OBJ)/StdGadgets.o $(OBJ)/StdGadgetSlider.o $(OBJ)/StdGadgetStatusBar.o $(OBJ)/StdImage.o $(OBJ)/StdTextFile.o \
	$(OBJ)/StdWildcard.o $(OBJ)/StdWindow.o $(OBJ)/Test.o $(OBJ)/Time.o $(OBJ)/Utils.o

All: $(OBJ) $(LIBRARY)

$(OBJ):
	@MakeDir $(OBJ)

$(LIBRARY): $(OBJECTS)
	@echo Creating library $@...
	$(AR) -r $@ $(OBJECTS)

$(OBJ)/%.o: %.cpp
	@echo Compiling $<...
	$(CC) $(CFLAGS) -o $(OBJ)/$*.o $<

clean:
	@Delete $(OBJ) all quiet
