
AR = @ar
CC = @g++
CFLAGS = -c -fno-asynchronous-unwind-tables -fno-strict-aliasing -Wall -Wextra -Wwrite-strings -DMUNGWALL_NO_LINE_TRACKING

ifdef DEBUG
	OBJ = Debug
	CFLAGS += -ggdb -D_DEBUG
else
	OBJ = Release
	CFLAGS += -O2
endif

LIBRARY = $(OBJ)/libStdFuncs.a

UNAME = $(shell uname)

ifeq ($(UNAME), AmigaOS)

OBJECTS = $(OBJ)/AmiMenus.o $(OBJ)/Args.o $(OBJ)/BaUtils.o $(OBJ)/Dir.o $(OBJ)/File.o $(OBJ)/Lex.o $(OBJ)/MungWall.o \
	$(OBJ)/StdApplication.o $(OBJ)/StdClipboard.o $(OBJ)/StdConfigFile.o $(OBJ)/StdCRC.o $(OBJ)/StdDialog.o \
	$(OBJ)/StdFileRequester.o $(OBJ)/StdFont.o $(OBJ)/StdGadgets.o $(OBJ)/StdGadgetLayout.o $(OBJ)/StdGadgetSlider.o \
	$(OBJ)/StdGadgetStatusBar.o $(OBJ)/StdImage.o $(OBJ)/StdPool.o $(OBJ)/StdRendezvous.o $(OBJ)/StdStringList.o \
	$(OBJ)/StdTextFile.o $(OBJ)/StdTime.o $(OBJ)/StdWildcard.o $(OBJ)/StdWindow.o $(OBJ)/Test.o $(OBJ)/Utils.o

else

OBJECTS = $(OBJ)/Args.o $(OBJ)/BaUtils.o $(OBJ)/Dir.o $(OBJ)/File.o $(OBJ)/Lex.o $(OBJ)/MungWall.o $(OBJ)/StdConfigFile.o \
	$(OBJ)/StdCRC.o $(OBJ)/StdPool.o $(OBJ)/StdRendezvous.o $(OBJ)/StdStringList.o $(OBJ)/StdTextFile.o $(OBJ)/StdTime.o \
	$(OBJ)/StdWildcard.o $(OBJ)/Test.o $(OBJ)/Utils.o

endif

All: $(OBJ) $(LIBRARY)

$(OBJ):
	@mkdir $(OBJ)

$(LIBRARY): $(OBJECTS)
	@echo Creating library $@...
	$(AR) -r $@ $(OBJECTS)

$(OBJ)/%.o: %.cpp
	@echo Compiling $<...
	$(CC) $(CFLAGS) -o $(OBJ)/$*.o $<

$(OBJ)/%.o: Amiga/%.cpp
	@echo Compiling $<...
	$(CC) $(CFLAGS) -o $(OBJ)/$*.o $<

clean:
	@rm -fr $(OBJ)
