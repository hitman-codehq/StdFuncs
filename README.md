# The Framework AKA StdFuncs

A Qt-inspired cross-platform framework with Amiga OS as a first-class citizen.  The Framework also has the distinction
of having the most unimaginative name for any cross-platform framework to date.

## Introduction

Inspired by Qt, this cross-platform framework was originally developed on Amiga OS and is aimed at making the development
of portable software easier.  It was initially developed to support the _Brunel_ text editor.  It enables building GUI
or command line software that can compile without modification on Amiga OS 3, OS4 and Windows, as well as any platform
that supports Qt 5.

The full list of platforms that this framework supports is:

- Amiga OS 3 m68k
- Amiga OS 4 PPC
- Windows 10 and above using the Win32 API
- Windows 10 and above using Qt
- Linux using Qt
- Mac OS using Qt

## Compiling

The Framework requires C++11 to compile.  It has been tested with GCC, clang and Microsoft Visual C++.  There are
several ways to compile the framework, depending on the target platform and desired use.

For Amiga OS, cross-compilation is recommended.  The framework is currently compiled using Bebbo's GCC 6.5.0b for
Amiga OS 3 (available from [here](https://github.com/bebbo/amiga-gcc)) and adtools GCC 8.4.0 for OS4 (available from
[here](https://github.com/sba1/adtools), although this one you have to compile yourself).

### Makefile

There is an old-school makefile that will compile the entire framework on Amiga OS and the non-GUI parts of the
framework on UNIX-like systems.

Simply run make.  Or, to compile a debug version:

```sh
make DEBUG=1
```

To cross-compile for Amiga OS, ensure that the compiler is in the PATH, export the PREFIX environment variable, and
then compile:

```sh
export PREFIX=m68k-amigaos-
make DEBUG=1
```

Note that while this software was initially developed natively on OS4, due to a lack of easily accessible C++11
compilers that run natively on that platform, it is now cross-compiled during development.  Attempting to compile
natively on OS4 may or may not work.

### Visual Studio Project File

There is a StdFuncs.vcxproj project file that can be used with Visual Studio or MS Build.  Supported targets are both
x86 and x64.  Note that there is no .sln file as the StdFuncs.vcxproj file is usually included by the .sln file
of the project that is using it.

Using the VS project file will result in a small and fast library being built, which targets the native Win32 API
or its 64-bit equivalent.

### Qt QMake File

This can be used from the command line in the usual manner:

```sh
qmake
make
```

Alternatively, it can be imported into Qt Creator.  When importing, go to the "Projects" page and in the general build
settings, ensure that the "Shadow-Build" checkbox is _not_ checked, to ensure that it is built into the same DebugQt
and ReleaseQt directories that are used from the command line build.  Otherwise, Qt creator will put it into a directory
with a name like ../build-StdFuncs-Desktop_Qt_6_5_1_clang_64bit-Debug and other projects in the Code HQ GitHub repo
won't be able to find the libraries when linking.

The QMake file _should_ work on all platforms that support Qt 5.

Using QMake file on Windows will result in a library being built that contains Qt-based code.  This library is _not_
compatible with the one built with the Visual Studio project file.

### CMake

This is useful if you would like to use Visual Studio Code for development.  Simply build in the usual CMake manner.
Note, however, that you need to let CMake know where to find the version of Qt that you are using.  For example, when
compiling on Mac OS:

```sh
export Qt6_DIR=~/Qt/5.15.2/clang_64/lib/cmake
mkdir build
cd build
cmake -GNinja ..
ninja
```

Compiling without Qt and the Qt6_DIR environment variable is still possible, but only the non-GUI components will be
compiled in this case.  This is useful when developing a project that does not have a GUI.

Alternatively, use the CMake plugin in Visual Studio Code and let it do the work for you.  The same rules regarding
the Qt6_DIR environment variable apply as for command line building.  Note that other projects in the Code HQ GitHub
repo assume that CMake has put the library into the StdFuncs/build directory.

## The Framework API

Full Doxygen documentation for The Framework can be found [here](https://hitman-codehq.github.io/StdFuncs/).
