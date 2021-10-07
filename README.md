# FCPP - FieldCalc++

An efficient C++14 implementation of the Pure Field Calculus, for fast and effective simulation of pervasive computing scenarios, and deployment on microcontroller architectures.

## References

- FCPP main website: [https://fcpp.github.io](https://fcpp.github.io).
- FCPP documentation: [http://fcpp-doc.surge.sh](http://fcpp-doc.surge.sh).
- FCPP presentation paper: [http://giorgio.audrito.info/static/fcpp.pdf](http://giorgio.audrito.info/static/fcpp.pdf).


## Setup

The next sections contain the setup instructions based on the CMake build system for the various supported OSs and virtual containers. Jump to the section dedicated to your system of choice and ignore the others.
For backward compatibility (and faster testing), the [Bazel](https://bazel.build) build system is also supported but not recommended: in particular, the OpenGL graphical user interface is not available with Bazel. In order to use Bazel instead of CMake for building, you have to install it and then substitute `./make.sh bazel` for `./make.sh` in the commands of the "Build" section.

### Windows

Pre-requisites:
- [Git Bash](https://gitforwindows.org) (for issuing unix-style commands)
- [MinGW-w64](http://mingw-w64.org) ([builds 8.1.0](http://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/installer/mingw-w64-install.exe/download))
- [CMake 3.9](https://cmake.org) (or higher)
- [Asymptote](http://asymptote.sourceforge.io) (for building simulation plots)
- [Doxygen](http://www.doxygen.nl) (for building the documentation)

During CMake installation, make sure you select to add `cmake` to the `PATH` (at least for the current user).
During MinGW installation, make sure you select "posix" threads (should be the default) and not "win32" threads. After installing MinGW, you need to add its path to the environment variable `PATH`. The default path should be:
```
C:\Program Files (x86)\mingw-w64\i686-8.1.0-posix-dwarf-rt_v6-rev0\mingw32\bin
```
but the actual path may vary depending on your installation.

### Linux

Pre-requisites:
- Xorg-dev package (X11)
- G++ 9 (or higher)
- CMake 3.9 (or higher)
- Asymptote (for building simulation plots)
- Doxygen (for building the documentation)

To install these packages in Ubuntu, type the following command:
```
sudo apt-get install xorg-dev g++ cmake asymptote doxygen
```
In Fedora, the `xorg-dev` package is not available. Instead, install the packages:
```
libX11-devel libXinerama-devel.x86_6 libXcursor-devel.x86_64 libXi-devel.x86_64 libXrandr-devel.x86_64 mesa-libGL-devel.x86_64
```

### MacOS

Pre-requisites:
- Xcode Command Line Tools
- CMake 3.9 (or higher)
- Asymptote (for building simulation plots)
- Doxygen (for building the documentation)

To install them, assuming you have the [brew](https://brew.sh) package manager, type the following commands:
```
xcode-select --install
brew install cmake asymptote doxygen
```

### Docker container

**Warning:** the graphical simulations are based on OpenGL, which is **not** available in the Docker container. Use this system for batch simulations only.

Download Docker from [https://www.docker.com](https://www.docker.com), then you can download the Docker container from GitHub by typing the following command in a terminal:
```
docker pull docker.pkg.github.com/fcpp/fcpp/container:1.0
```
Alternatively, you can build the container yourself with the following command:
```
docker build -t docker.pkg.github.com/fcpp/fcpp/container:1.0 .
```
Once you have the Docker container locally available, type the following command to enter the container:
```
docker run -it --volume $PWD:/fcpp --workdir /fcpp docker.pkg.github.com/fcpp/fcpp/container:1.0 bash
```
and the following command to exit it:
```
exit
```
In order to properly link the executables in Docker, you may need to add the `-pthread` option (substitute `-O` for `-O -pthread` below).

### Vagrant container

**Warning:** the graphical simulations are based on OpenGL, which is **not** available in the Vagrant container. Use this system for batch simulations only.

Download Vagrant from [https://www.vagrantup.com](https://www.vagrantup.com) and VirtualBox from [https://www.virtualbox.org](https://www.virtualbox.org), then type the following commands in a terminal to enter the Vagrant container:
```
vagrant up
vagrant ssh
cd fcpp
```
and the following commands to exit it:
```
exit
vagrant halt
```

### Virtual Machines

If you use a VM with a graphical interface, refer to the section for the operating system installed on it.

**Warning:** the graphical simulations are based on OpenGL, and common Virtual Machine software (e.g., VirtualBox) has faulty support for OpenGL. If you rely on a virtual machine for graphical simulations, it might work provided that you select hardware virtualization (as opposed to software virtualization). However, it is recommended to use the native OS whenever possible.


## Build

### Documentation

You should be able to build the same documentation [available oline](http://fcpp-doc.surge.sh) with the following command:
```
./make.sh doc
```
issued from the `src` subfolder.

### Testing

You can run all the automated tests (through the [googletest](https://github.com/google/googletest) framework) with the following command:
```
./make.sh test all
```
issued from the `src` subfolder. In order to test a specific target, substitute `all` with (a substring of) the target of your choice.

### Plots

FCPP includes a [plot generation tool](http://fcpp-doc.surge.sh/plot_8hpp.html), which can be integrated with the [logger](http://fcpp-doc.surge.sh/structfcpp_1_1component_1_1logger.html) component. The tool produces code that can be compiled into vector graphics through [Asymptote](http://asymptote.sourceforge.io) and a provided custom [header](https://github.com/fcpp/fcpp/blob/master/src/extras/plotter/plot.asy). Install Asymptote if you plan to use it.

### Execution

In order to build a project based on FCPP, it is recommended to start following the [sample](https://github.com/fcpp/fcpp-sample-project) or [exercises](https://github.com/fcpp/fcpp-exercises) projects. In short, add this repository as a sub-module of your repository, add a `make.sh` script forwarding its arguments to the inner `fcpp/src/make.sh` script, add your code which uses the library, then declare the executable sources appropriately in a `CMakeLists.txt` as `fcpp_target` (see the aforementioned repositories for reference). Then you will be able to run your targets with the following command:
```
./make.sh [gui] run [-O] [target]
```
You can omit the `gui` argument if you don't need the graphical user interface; or omit the `-O` argument for a debug build (instead of an optimised build). If you plan to produce plots, you will also need to copy the `src/extras/plotter/plot.asy` file into a `plot/` subfolder of your repository.

### Graphical User Interface

If you write and launch your own graphical simulation, a window will open displaying the simulation scenario, initially still: you can start running the simulation by pressing `P` (current simulated time is displayed in the bottom-left corner). While the simulation is running, network statistics may be periodically printed in the console, and be possibly aggregated in form of an Asymptote plot at simulation end. You can interact with the simulation through the following keys:
- `Esc` to end the simulation
- `P` to stop/resume
- `O`/`I` to speed-up/slow-down simulated time
- `L` to show/hide connection links between nodes
- `G` to show/hide the grid on the reference plane and node pins
- `M` enables/disables the marker for selecting nodes
- `left-click` on a selected node to open a window with node details
- `C` resets the camera to the starting position
- `Q`,`W`,`E`,`A`,`S`,`D` to move the simulation area along orthogonal axes
- `right-click`+`mouse drag` to rotate the camera
- `mouse scroll` for zooming in and out
- `left-shift` added to the camera commands above for precision control
- any other key will show/hide a legenda displaying this list
Hovering on a node will also display its UID in the top-left corner.


## Developers

### Status Badges

#### Stable Branch
[![Build Status](https://github.com/fcpp/fcpp/actions/workflows/main.yml/badge.svg?branch=master)](https://github.com/fcpp/fcpp/actions?query=branch%3Amaster)
[![Codacy](https://api.codacy.com/project/badge/Grade/90634407d674499cb62da7d7d74e8b42)](https://app.codacy.com/gh/fcpp/fcpp?utm_source=github.com&utm_medium=referral&utm_content=fcpp/fcpp&utm_campaign=Badge_Grade_Dashboard)

#### Development Branch
[![Build Status](https://github.com/fcpp/fcpp/actions/workflows/main.yml/badge.svg?branch=dev)](https://github.com/fcpp/fcpp/actions?query=branch%3Adev)

### Authors

#### Main Developer

- [Giorgio Audrito](http://giorgio.audrito.info/#!/research)

#### Contributors

- [Gianmarco Rampulla](https://github.com/denoide1)
- [Luigi Rapetta](https://github.com/rapfamily4)
- [Gianluca Torta](http://www.di.unito.it/~torta)
