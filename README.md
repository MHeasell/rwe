# Robot War Engine

An open-source real-time strategy game engine
that is highly compatible with Total Annihilation data files.

## Build Status

[![AppVeyor build status](https://ci.appveyor.com/api/projects/status/43armkvatrbaiur5/branch/master?svg=true)](https://ci.appveyor.com/project/MHeasell/rwe/branch/master)
[![Travis CI build Status](https://travis-ci.org/MHeasell/rwe.svg?branch=master)](https://travis-ci.org/MHeasell/rwe)

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

## How to Play

1. Create the folder `%AppData%/RWE/Data` and copy your TA data files to it (.hpi, .ufo, rev31.gp3, etc.)
2. Run rwe.exe (if you used the installer, RWE will be in your start menu items)

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
Finally, go to `CMake > Debug from Install Folder > rwe.exe`
to build and launch RWE.

### Windows with MSYS2

Download and install MSYS2 (http://www.msys2.org/)

Install the required packages:

    pacman -S make mingw-w64-x86_64-cmake mingw-w64-x86_64-toolchain
    pacman -S \
      mingw-w64-x86_64-boost \
      mingw-w64-x86_64-SDL2 \
      mingw-w64-x86_64-SDL2_image \
      mingw-w64-x86_64-SDL2_mixer \
      mingw-w64-x86_64-SDL2_net \
      mingw-w64-x86_64-glew \
      mingw-w64-x86_64-smpeg2

Open the `MSYS MinGW 64-bit` terminal and do the following
to generate and build the project:

    cd /path/to/rwe
    mkdir build
    cd build
    cmake .. -G 'Unix Makefiles'
    make

Once built, launch RWE from the top-level directory:

    cd /path/to/rwe
    build/rwe.exe

### Ubuntu

Install the necessary packages:

    sudo add-apt-repository ppa:ubuntu-toolchain-r/test
    sudo apt-get update
    sudo apt-get install \
      gcc-7 \
      g++-7 \
      libboost-dev \
      libboost-filesystem-dev \
      libsdl2-dev \
      libsdl2-image-dev \
      libsdl2-net-dev \
      libsdl2-mixer-dev \
      libglew-dev

Ensure you have a recent version of CMake (3.8+).
The one provided by your package manager may not be new enough.

Build the code:

    cd /path/to/rwe
    mkdir build
    cd build
    export CC=gcc-7 CXX=g++-7
    cmake .. -G 'Unix Makefiles'
    make

Once built, launch RWE from the top-level directory:

    cd /path/to/rwe
    build/rwe
