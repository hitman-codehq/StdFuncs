
TARGET = StdFuncs
TEMPLATE = lib
CONFIG += debug_and_release staticlib warn_on
CONFIG -= exceptions rtti

# Some tricks to get rid of most of the contents of the largish .eh_frame section
QMAKE_CXXFLAGS += -fno-asynchronous-unwind-tables

build_pass:CONFIG(debug, debug|release) {
	DEFINES += _DEBUG
	DESTDIR = DebugQt
}

build_pass:CONFIG(release, debug|release) {
	DESTDIR = ReleaseQt
}

# Ensure that all temporary files are generated in the correct location as setting DESTDIR
# by itself is not enough.  The second line is also a workaround to prevent qmake from
# generating empty "debug" and "release" directories

OBJECTS_DIR = $$DESTDIR
MOC_DIR = $$OBJECTS_DIR

DEFINES -= UNICODE

SOURCES += Args.cpp BaUtils.cpp Dir.cpp File.cpp Lex.cpp MungWall.cpp \
	StdApplication.cpp StdClipboard.cpp StdConfigFile.cpp StdDialog.cpp StdFileRequester.cpp \
	StdFont.cpp StdGadgets.cpp StdGadgetLayout.cpp StdGadgetSlider.cpp StdGadgetStatusBar.cpp \
	StdImage.cpp StdPool.cpp StdTextFile.cpp StdWildcard.cpp StdWindow.cpp Test.cpp \
	Time.cpp Utils.cpp

HEADERS += Args.h BaUtils.h Dir.h File.h Lex.h MungWall.h StdApplication.h StdClipboard.h \
	StdConfigFile.h StdDialog.h StdFileRequester.h StdFont.h StdFuncs.h StdGadgets.h \
	StdImage.h StdList.h StdPool.h StdReaction.h StdTextFile.h StdWildcard.h StdWindow.h \
	Test.h Time.h Utils.h

HEADERS += Qt/StdGadgetSlider.h
HEADERS += Qt/StdWindow.h
