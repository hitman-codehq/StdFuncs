
CFLAGS = -c -fno-asynchronous-unwind-tables -fno-exceptions -fno-strict-aliasing -fpermissive -std=gnu++14 -Wall -Wextra -Wwrite-strings
CFLAGS_EX = -c -fno-asynchronous-unwind-tables -fno-strict-aliasing -fpermissive -std=gnu++14 -Wall -Wextra -Wwrite-strings
IFLAGS = -I. -D__USE_INLINE__

ifdef PREFIX
	AR = @$(PREFIX)ar
	CC = @$(PREFIX)g++
	LD = @$(PREFIX)g++
else
	AR = @ar
	CC = @g++
	LD = @g++
endif

ifdef DEBUG
	OBJ = Debug
	CFLAGS += -ggdb -D_DEBUG -DMUNGWALL_NO_LINE_TRACKING
else
	OBJ = Release
	CFLAGS += -O2
endif

ifdef PREFIX
	ifeq ($(PREFIX), ppc-amigaos-)
		CFLAGS += -athread=native
		LFLAGS += -athread=native
		OBJ := $(OBJ)_OS4
	else
		AUTO_LIBRARY = $(OBJ)/libauto.a
	endif
endif

LIBRARY = $(OBJ)/libStdFuncs.a

ifdef PREFIX
	OBJECTS = $(OBJ)/AmiMenus.o $(OBJ)/Args.o $(OBJ)/Dir.o $(OBJ)/File.o $(OBJ)/FileUtils.o $(OBJ)/Lex.o $(OBJ)/MungWall.o \
		$(OBJ)/RemoteDir.o $(OBJ)/RemoteFile.o $(OBJ)/RemoteFileUtils.o \
		$(OBJ)/StdApplication.o $(OBJ)/StdCharConverter.o $(OBJ)/StdClipboard.o $(OBJ)/StdConfigFile.o $(OBJ)/StdCRC.o $(OBJ)/StdDialog.o \
		$(OBJ)/StdFileRequester.o $(OBJ)/StdFont.o $(OBJ)/StdGadgets.o $(OBJ)/StdGadgetLayout.o $(OBJ)/StdGadgetSlider.o \
		$(OBJ)/StdGadgetStatusBar.o $(OBJ)/StdGadgetTree.o $(OBJ)/StdImage.o $(OBJ)/StdPool.o $(OBJ)/StdRendezvous.o $(OBJ)/StdSocket.o \
		$(OBJ)/StdStringList.o $(OBJ)/StdTextFile.o $(OBJ)/StdTime.o $(OBJ)/StdWildcard.o $(OBJ)/StdWindow.o $(OBJ)/Test.o \
		$(OBJ)/Utils.o

	ifneq ($(PREFIX), ppc-amigaos-)
		AUTO_OBJECTS = $(OBJ)/AutoAsl.o $(OBJ)/AutoBitMap.o $(OBJ)/AutoCheckBox.o $(OBJ)/AutoDataTypes.o $(OBJ)/AutoDiskfont.o \
			$(OBJ)/AutoGadTools.o $(OBJ)/AutoGfx.o $(OBJ)/AutoIcon.o $(OBJ)/AutoIFFParse.o $(OBJ)/AutoIntuition.o $(OBJ)/AutoKeymap.o \
			$(OBJ)/AutoLabel.o $(OBJ)/AutoLayout.o $(OBJ)/AutoListBrowser.o $(OBJ)/AutoScroller.o $(OBJ)/AutoString.o \
			$(OBJ)/AutoUtility.o $(OBJ)/AutoWindow.o $(OBJ)/SafeOpenLibrary.o

		OBJECTS += $(OBJ)/OS4Support.o
	endif
else
	OBJECTS = $(OBJ)/Args.o $(OBJ)/StdCharConverter.o $(OBJ)/Dir.o $(OBJ)/File.o $(OBJ)/FileUtils.o $(OBJ)/Lex.o $(OBJ)/MungWall.o \
		$(OBJ)/RemoteDir.o $(OBJ)/RemoteFile.o $(OBJ)/RemoteFileUtils.o \
		$(OBJ)/StdConfigFile.o $(OBJ)/StdCRC.o $(OBJ)/StdPool.o $(OBJ)/StdRendezvous.o $(OBJ)/StdSocket.o $(OBJ)/StdStringList.o \
		$(OBJ)/StdTextFile.o $(OBJ)/StdTime.o $(OBJ)/StdWildcard.o $(OBJ)/Test.o $(OBJ)/Utils.o
endif

OBJECTS += $(OBJ)/Handler.o

all: $(OBJ) $(LIBRARY) $(AUTO_LIBRARY)

$(OBJ):
	@mkdir $(OBJ)

$(LIBRARY): $(OBJECTS)
	@echo Creating library $@...
	$(AR) -rs $@ $(OBJECTS)

ifdef PREFIX

$(AUTO_LIBRARY): $(AUTO_OBJECTS)
	@echo Creating library $@...
	$(AR) -rs $@ $(AUTO_OBJECTS)

endif

$(OBJ)/%.o: %.cpp
	@echo Compiling $<...
	$(CC) $(CFLAGS) $(IFLAGS) -o $(OBJ)/$*.o $<

$(OBJ)/%.o: Amiga/%.cpp
	@echo Compiling $<...
	$(CC) $(CFLAGS) $(IFLAGS) -o $(OBJ)/$*.o $<

$(OBJ)/%.o: Auto/%.c
	@echo Compiling $<...
	$(CC) $(CFLAGS) $(IFLAGS) -o $(OBJ)/$*.o $<

$(OBJ)/%.o: Yggdrasil/%.cpp
	@echo Compiling $<...
	$(CC) $(CFLAGS) $(IFLAGS) -o $(OBJ)/$*.o $<

$(OBJ)/StdSocket.o: StdSocket.cpp
	@echo Compiling $<...
	$(CC) $(CFLAGS_EX) $(IFLAGS) -o $(OBJ)/StdSocket.o $<

clean:
	@rm -fr $(OBJ)
