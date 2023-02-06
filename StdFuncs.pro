
TARGET = StdFuncs
TEMPLATE = lib
CONFIG += debug_and_release staticlib warn_on
CONFIG -= rtti

# Some tricks to get rid of most of the contents of the largish .eh_frame section
gcc:QMAKE_CXXFLAGS += -fno-asynchronous-unwind-tables

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

SOURCES += Args.cpp Dir.cpp File.cpp FileUtils.cpp Lex.cpp MungWall.cpp \
    RemoteDir.cpp RemoteFile.cpp RemoteFileUtils.cpp Yggdrasil/Handler.cpp \
	StdApplication.cpp StdCharConverter.cpp StdClipboard.cpp StdConfigFile.cpp StdCRC.cpp StdDialog.cpp StdFileRequester.cpp \
	StdFont.cpp StdGadgets.cpp StdGadgetLayout.cpp StdGadgetSlider.cpp StdGadgetStatusBar.cpp StdGadgetTree.cpp \
	StdImage.cpp StdPool.cpp StdRendezvous.cpp StdSocket.cpp StdStringList.cpp StdTextFile.cpp StdTime.cpp \
	StdWildcard.cpp StdWindow.cpp Test.cpp Utils.cpp

SOURCES += Qt/QtAction.cpp Qt/QtGadgetSlider.cpp Qt/QtGadgetTree.cpp Qt/QtLocalSocket.cpp Qt/QtWindow.cpp

HEADERS += Args.h Dir.h File.h FileUtils.h Lex.h MungWall.h StdApplication.h StdCharConverter.h StdClipboard.h \
	StdConfigFile.h StdDialog.h StdFileRequester.h StdFont.h StdFuncs.h StdGadgets.h StdImage.h StdList.h \
	StdPool.h StdReaction.h StdRendezvous.h StdSocket.h StdStringList.h StdTextFile.h StdTime.h StdWildcard.h \
	StdWindow.h Test.h Utils.h

HEADERS += Qt/QtAction.h Qt/QtGadgetSlider.h Qt/QtGadgetTree.h Qt/QtLocalSocket.h Qt/QtWindow.h
