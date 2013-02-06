
TARGET = StdFuncs
TEMPLATE = lib
CONFIG += debug_and_release staticlib warn_on

build_pass:CONFIG(debug, debug|release) {
	DESTDIR = Debug_Qt
	OBJECTS_DIR = Debug_Qt
}

build_pass:CONFIG(release, debug|release) {
	DESTDIR = Release_Qt
	OBJECTS_DIR = Release_Qt
}

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

HEADERS += Qt/StdWindow.h
