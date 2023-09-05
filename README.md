# Robot War Engine

An open-source real-time strategy game engine
that is highly compatible with Total Annihilation data files.

## Build Status

[![AppVeyor build status](https://ci.appveyor.com/api/projects/status/43armkvatrbaiur5/branch/master?svg=true)](https://ci.appveyor.com/project/MHeasell/rwe/branch/master)
[![GitHub Build Status](https://github.com/MHeasell/rwe/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/MHeasell/rwe/actions/workflows/build.yml)

## Download

There is currently no stable release.
If you would like try out the latest, bleeding edge version,
the latest zip file and/or installer is available at:

https://ci.appveyor.com/project/MHeasell/rwe/branch/master

To find the files, click "Environment: RWE\_COMPILER=MSYS; Configuration: Release", then "Artifacts".

RWE is currently only available on Windows,
however the code is also built and tested on Linux.
Official Linux binaries will be available when the project reaches a stable version.

Source code is hosted on Github:

https://github.com/MHeasell/rwe

## How to Install

1. Create the folder `%AppData%/RWE/Data` and copy your TA data files to it (.hpi, .ufo, rev31.gp3, etc.)
2. Run rwe.exe (if you used the installer, RWE will be in your start menu items)

## How to Play

General:
- Scroll through the map using the arrow keys (UP, DOWN, LEFT, RIGHT).

Units:
- Left click to select units.
- Right click to move units to the clicked area on the map.
- To deselect units, left click on the map itself, off the unit.
- To attack, with the unit selected, press A and select where to attack.
- To stop units attacking, select them and press S.

Debugging:
- To show the global debug menu, press F11.
  This contains debugging options that are relevant globally,
  regardless of whether the engine is in-game or in a menu.
- To show the in-game debug menu, press F10.
  This can only be done while loaded into a game.
  This contains debugging options specific to the in-game world
  such as spawning units.

## Development Status

Progress updates are posted to a thread on the TAUniverse forums,
usually once a week. See:

http://www.tauniverse.com/forum/showthread.php?t=45555

## How to Build

First fetch the source code:

    git clone https://github.com/MHeasell/rwe.git
    cd rwe
    git submodule update --init --recursive

Then follow the instructions for your platform.

### Windows with Visual Studio

Get the RWE MSVC libraries bundle.
A python 3 script is provided that will do this for you:

    cd /path/to/rwe
    python ./fetch-msvc-libs.py

Alternatively you can double-click the script in Explorer
and it should run that way.

If the script completes successfully you should have a `libs/_msvc` folder containing various library files.

Now open Visual Studio 2017, go to `File > Open > CMake...`
and select the `CMakeLists.txt` in the `rwe` folder.
Choose `x64-Debug` in the build configuration dropdown.
Finally, go to `CMake > Debug from Build Folder > rwe.exe`
to build and launch RWE.

### Windows with MSYS2

Download and install MSYS2 (http://www.msys2.org/)

Choose to run at the end of install, and in the terminal that opens install the required packages:

    pacman -S make unzip mingw-w64-x86_64-cmake mingw-w64-x86_64-toolchain
    pacman -S \
      mingw-w64-x86_64-boost \
      mingw-w64-x86_64-SDL2 \
      mingw-w64-x86_64-SDL2_image \
      mingw-w64-x86_64-SDL2_mixer \
      mingw-w64-x86_64-glew \
      mingw-w64-x86_64-smpeg2 \
      mingw-w64-x86_64-zlib \
      mingw-w64-x86_64-libpng \
      git

Close the terminal, and open the `MSYS2 MinGW64` terminal
(a shortcut should be in the start menu under MSYS2 - the default install points to C:\msys64\mingw64.exe starting in C:\msys64 directory)

Compile protobuf:

    cd /path/to/rwe
    cd libs
    ./build-protobuf.sh

Generate and build the project:

    cd /path/to/rwe
    mkdir build
    cd build
    cmake .. -G 'Unix Makefiles'
    make

Once built, launch RWE from the top-level directory:

    cd /path/to/rwe
    build/rwe.exe

### Windows with Visual Studio Code
Install Visual Studio Code, and open it. Under extensions, search for C/C++, and install the C/C++ Extension Pack from Microsoft, which includes CMake Tools and C/C++ development extensions. Now VS Code should be able to recognize C/C++ and compilers, and understand the CMake build configuration used by RWE.

#### Open the project in VS Code:
File > Open Folder, choose the root directory of the RWE repository (where CMakeLists.txt is). At the bottom of the VS Code window you should see CMake: [Debug]: Ready and probably No Kit Selected. Click No Kit Selected to choose which compiler to use. VS Code should auto-detect compilers on your machine, so if none are listed here, install Visual Studio or MSYS2 first and try again.
Once a toolset is selected, CMake will configure itself for the project, with its output in the OUTPUT window. When that's done, you should be able to build by clicking the Build button there on the bottom or hit F7.

### Ubuntu

Install the necessary packages:

    sudo add-apt-repository ppa:ubuntu-toolchain-r/test
    sudo apt-get update
    sudo apt-get install \
      gcc-7 \
      g++-7 \
      libboost-dev \
      libboost-filesystem-dev \
      libboost-program-options-dev \
      libsdl2-dev \
      libsdl2-image-dev \
      libsdl2-mixer-dev \
      libglew-dev \
      zlib1g-dev \
      libpng-dev

Ensure you have a recent version of CMake (3.8+).
The one provided by your package manager may not be new enough.
Here's how you might install CMake:

    wget 'https://cmake.org/files/v3.8/cmake-3.8.2-Linux-x86_64.tar.gz'
    tar -xf cmake-3.8.2-Linux-x86_64.tar.gz
    export CMAKE_MODULE_PATH=$(pwd)/cmake-3.8.2-Linux-x86_64/share/cmake-3.8/Modules
    export PATH=$(pwd)/cmake-3.8.2-Linux-x86_64/bin:$PATH

Compile protobuf:

    cd /path/to/rwe
    cd libs
    ./build-protobuf.sh

Now build the code:

    cd /path/to/rwe
    mkdir build
    cd build
    export CC=gcc-7 CXX=g++-7
    cmake .. -G 'Unix Makefiles'
    make

Install some TA data files (.hpi, .ufo, .ccx, rev31.gp3, etc.)
to your local data directory:

    mkdir -p $HOME/.rwe/Data
    cp /path/to/totala/*.hpi $HOME/.rwe/Data
    cp /path/to/totala/*.ufo $HOME/.rwe/Data
    cp /path/to/totala/*.ccx $HOME/.rwe/Data
    cp /path/to/totala/*.gpf $HOME/.rwe/Data
    cp /path/to/totala/*.gp3 $HOME/.rwe/Data

Alternatively you can symlink `.rwe/Data` to your TA directory.

Finally, launch RWE from the top-level project directory:

    cd /path/to/rwe
    build/rwe

## The Launcher

The launcher application provides the multiplayer lobby for RWE.

### Motivation

Multiplayer is implemented via a separate application
rather than by recreating the original in-game multiplayer lobby
for the following reasons.

In the future, the goal is to improve the multiplayer lobby experience
beyond what is supported by the orignal TA multiplayer lobby.
The RWE launcher will eventually support managing installed mods,
and will ensure that all players in a multiplayer game
have the same mods enabled before RWE is launched.
It is impractical to achieve this with an implementation
of the original multiplayer lobby inside RWE
because RWE is primarily driven by the supplied game data,
and this data dictates the design and layout of screens,
as well as what screens are even available.
An interface that manages mods must exist and work
independently of any individual mod's game data.
In this context, original TA data is also a mod
that may or may not be available.

The engine codebase is already quite large and unwieldy.
It is written in C++ for performance,
but this does not provide a great development experience.
The multiplayer lobby is not performance-critical,
and given what we want to accomplish in future,
it seems more practical to use a different platform
that provides a better developer experience
and is more suited to building traditional GUI applications.

### Technology Choices

The launcher is an Electron application written in TypeScript
with React and Redux.

### How to Build and Run for Development

First go to the launcher directory and install the required npm modules.

    cd path/to/rwe/launcher
    npm install

Then start the webpack dev server

    npm run server

The server will start up and stay running until interrupted.
It serves the compiled application code during development
and provides hot-reloading.

Open another terminal session for the next step,
launching the application itself.

For development, you will need to set the `RWE_HOME` environment variable
to a directory containing the `rwe` and `rwe_bridge` programs.

    export RWE_HOME=/my/installed/rwe/dir

Now you can start the application.

    npm start

The application window should now appear.

The launcher expects to talk to a master server
that manages the list of publicly available games.
In development, the launcher will try to connect
to a master server instance running on the local machine.
To start a master server instance locally,
open another terminal session and run the following:

    npm run master-server

The master server will start up and stay running until interrupted.
Any running launcher instances should automatically connect
to the master server.

### How To Package

For releases, the launcher must be bundled up
as a complete Electron application.
To do this, run the following:

    npm run package

The built application will be written out to a subdirectory
whose name depends on the target platform.
For example, for 64-bit Windows the directory name is
`rwe-launcher-win32-x64`.
