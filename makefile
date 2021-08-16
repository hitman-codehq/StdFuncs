
CFLAGS = -c -fno-asynchronous-unwind-tables -fno-exceptions -fno-strict-aliasing -fpermissive -std=gnu++14 -Wall -Wextra -Wwrite-strings
CFLAGS_EX = -c -fno-asynchronous-unwind-tables -fno-strict-aliasing -fpermissive -std=gnu++14 -Wall -Wextra -Wwrite-strings
IFLAGS = -D__USE_INLINE__

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

UNAME = $(shell uname)

ifeq ($(UNAME), CYGWIN_NT-10.0)

CFLAGS += -athread=native
LFLAGS += -athread=native
OBJ := $(OBJ)_OS4

else

AUTO_LIBRARY = $(OBJ)/libauto.a

endif

LIBRARY = $(OBJ)/libStdFuncs.a

ifdef PREFIX

OBJECTS = $(OBJ)/AmiMenus.o $(OBJ)/Args.o $(OBJ)/BaUtils.o $(OBJ)/Dir.o $(OBJ)/File.o $(OBJ)/Lex.o $(OBJ)/MungWall.o \
	$(OBJ)/StdApplication.o $(OBJ)/StdClipboard.o $(OBJ)/StdConfigFile.o $(OBJ)/StdCRC.o $(OBJ)/StdDialog.o \
	$(OBJ)/StdFileRequester.o $(OBJ)/StdFont.o $(OBJ)/StdGadgets.o $(OBJ)/StdGadgetLayout.o $(OBJ)/StdGadgetSlider.o \
	$(OBJ)/StdGadgetStatusBar.o $(OBJ)/StdImage.o $(OBJ)/StdPool.o $(OBJ)/StdRendezvous.o $(OBJ)/StdSocket.o \
	$(OBJ)/StdStringList.o $(OBJ)/StdTextFile.o $(OBJ)/StdTime.o $(OBJ)/StdWildcard.o $(OBJ)/StdWindow.o $(OBJ)/Test.o \
	$(OBJ)/Utils.o

else

OBJECTS = $(OBJ)/Args.o $(OBJ)/BaUtils.o $(OBJ)/Dir.o $(OBJ)/File.o $(OBJ)/Lex.o $(OBJ)/MungWall.o $(OBJ)/StdConfigFile.o \
	$(OBJ)/StdCRC.o $(OBJ)/StdPool.o $(OBJ)/StdRendezvous.o $(OBJ)/StdSocket.o $(OBJ)/StdStringList.o $(OBJ)/StdTextFile.o \
	$(OBJ)/StdTime.o $(OBJ)/StdWildcard.o $(OBJ)/Test.o $(OBJ)/Utils.o

endif

ifneq ($(UNAME), CYGWIN_NT-10.0)

AUTO_OBJECTS = $(OBJ)/auto_asl.o $(OBJ)/auto_bitmap.o $(OBJ)/auto_datatypes.o $(OBJ)/auto_checkbox.o $(OBJ)/auto_diskfont.o \
	$(OBJ)/auto_gadtools.o $(OBJ)/auto_graphics.o $(OBJ)/auto_icon.o $(OBJ)/auto_iffparse.o $(OBJ)/auto_intuition.o \
	$(OBJ)/auto_keymap.o $(OBJ)/auto_label.o $(OBJ)/auto_layout.o $(OBJ)/auto_listbrowser.o  $(OBJ)/auto_scroller.o \
	$(OBJ)/auto_string.o $(OBJ)/auto_utility.o $(OBJ)/auto_window.o

OBJECTS += $(OBJ)/OS4Support.o

endif

all: $(OBJ) $(LIBRARY) $(AUTO_LIBRARY)

$(OBJ):
	@mkdir $(OBJ)

$(LIBRARY): $(OBJECTS)
	@echo Creating library $@...
	$(AR) -rs $@ $(OBJECTS)

$(AUTO_LIBRARY): $(AUTO_OBJECTS)
	@echo Creating library $@...
	$(AR) -rs $@ $(AUTO_OBJECTS)

$(OBJ)/%.o: %.cpp
	@echo Compiling $<...
	$(CC) $(CFLAGS) $(IFLAGS) -o $(OBJ)/$*.o $<

$(OBJ)/%.o: Amiga/%.cpp
	@echo Compiling $<...
	$(CC) $(CFLAGS) $(IFLAGS) -o $(OBJ)/$*.o $<

$(OBJ)/%.o: Auto/%.c
	@echo Compiling $<...
	$(CC) $(CFLAGS) $(IFLAGS) -o $(OBJ)/$*.o $<

$(OBJ)/StdSocket.o: StdSocket.cpp
	@echo Compiling $<...
	$(CC) $(CFLAGS_EX) $(IFLAGS) -o $(OBJ)/StdSocket.o $<

clean:
	@rm -fr $(OBJ)
