# FCPP - FieldCalc++

An efficient C++14 implementation of the Pure Field Calculus, for fast and effective simulation of pervasive computing scenarios, and deployment on microcontroller architectures.

## References

- FCPP main website: [https://fcpp.github.io](https://fcpp.github.io).
- FCPP documentation: [http://fcpp-doc.surge.sh](http://fcpp-doc.surge.sh).
- FCPP presentation paper: [http://giorgio.audrito.info/static/fcpp.pdf](http://giorgio.audrito.info/static/fcpp.pdf).


## Batch Simulation

Batch simulations are preferably executed through the [Bazel](https://bazel.build) build system, and can be either done in a virtual environment for an easier setup, or in the native OS for increased performance.

### Vagrant

Download Vagrant from [https://www.vagrantup.com](https://www.vagrantup.com), then type the following commands in a terminal, starting from the `src` folder of the repository:
```
vagrant up
vagrant ssh
cd fcpp
./make.sh test all
```
Then you should get output about building and testing the whole library (in the Vagrant virtual machine). After that you can exit and stop the virtual machine through:
```
exit
vagrant halt
```

### Docker

Download Docker from [https://www.docker.com](https://www.docker.com), then download the Docker container from GitHub by typing the following command in a terminal:
```
docker pull docker.pkg.github.com/fcpp/fcpp/container:1.0
```
Alternatively, you can build the container yourself with the following command:
```
docker build -t docker.pkg.github.com/fcpp/fcpp/container:1.0 .
```
Once you have the Docker container locally available, type the following commands from the `src` folder of the repository:
```
docker run -it --volume $PWD:/fcpp --workdir /fcpp docker.pkg.github.com/fcpp/fcpp/container:1.0 bash
./make.sh test all
```
Then you should get output about building and testing the whole library (in the Docker container). After that you can exit and stop the container through:
```
exit
```

### Custom Build

In order to get started on your machine you need the following installed:

- [Bazel](https://bazel.build) (tested with version 2.1.0)
- [GCC](https://gcc.gnu.org) (tested with version 9.2.0) or [Clang](https://clang.llvm.org) (tested with Apple Clang 12.0.5)
- [Doxygen](http://www.doxygen.nl)

In the `src` folder, you should be able to run `./make.sh test all`, getting output about building and testing the whole library.

### Documentation

Regardless of the method chosen above, you should be able to build the same documentation [available oline](http://fcpp-doc.surge.sh) with the following command:
```
./make.sh doc
```

### Plots

FCPP includes a [plot generation tool](http://fcpp-doc.surge.sh/plot_8hpp.html), which can be integrated with the [logger](http://fcpp-doc.surge.sh/structfcpp_1_1component_1_1logger.html) component. The tool produces code that can be compiled into vector graphics through [Asymptote](http://asymptote.sourceforge.io) and a provided custom [header](https://github.com/fcpp/fcpp/blob/master/src/extras/plotter/plot.asy). Install Asymptote if you plan to use it.


## Graphical Simulation

The OpenGL-based graphical simulations can only be built through the [CMake](https://cmake.org) build system in the native OS. Common Virtual Machine software (e.g., VirtualBox) has faulty support for OpenGL, hence running the graphical experiments in a VM is not supported: it may work for you, but it is not recommended.

### Windows

Pre-requisites:
- [Git Bash](https://gitforwindows.org) (for issuing unix-style commands)
- [MinGW-w64](http://mingw-w64.org) (GCC 11.2.0 or higher)
- [CMake 3.18](https://cmake.org) (or higher)
- [Asymptote](http://asymptote.sourceforge.io) (for building the plots)

It is recommended to install MinGW-w64 and CMake through [MSYS2](https://www.msys2.org/) in order to get the latest version of MinGW-w64's GCC and CMake. To do so:
- Download and install MSYS2.
- Run "MSYS2 MSYS" from the start menu; a terminal will appear.
- Run `pacman -Syu`; a restart of all MSYS2 processes is required at the end of the update.
- Run "MSYS2 MSYS" again, and run `pacman -Su`.
- Run `pacman -S --needed base-devel mingw-w64-x86_64-toolchain` to install the MinGW-w64 toolchain.
- Run `pacman -S mingw-w64-x86_64-cmake` to install CMake.
- Run `pacman -S mingw-w64-x86_64-make` to install MinGW-w64's make tool (used by CMake).

After the installation of these packages, make sure to add their path to the `PATH` environment variable (e.g., by editing the `.bashrc` file in your home). They should reside in MSYS2's installation folder as such:
```
C:\msys64\mingw64\bin
```
but the actual path may vary depending on your installation (GCC's and CMake's binaries are already in `PATH` if you execute the "MSYS2 MinGW x64" shortcut from the start menu). Then, you should be able to build the whole library with CMake through:
```
./make.sh gui windows
```

### Linux

Pre-requisites:
- Xorg-dev package (X11)
- G++ 9 (or higher)
- CMake 3.9 (or higher)

To install these packages in Ubuntu, type the following command:
```
sudo apt-get install xorg-dev g++ cmake asymptote
```
Then, you should be able to build the whole library with CMake through:
```
./make.sh gui unix
```

### MacOS

Pre-requisites:
- Xcode Command Line Tools
- CMake 3.9 (or higher)

To install them, assuming you have the [brew](https://brew.sh) package manager, type the following commands:
```
xcode-select --install
brew install cmake asymptote
```
Then, you should be able to build the whole library with CMake through:
```
./make.sh gui unix
```

### User Interface

If you write and launch your own graphical simulation, a window will open displaying the simulation scenario, initially still: you can start running the simulation by pressing `P` (current simulated time is displayed in the bottom-left corner). While the simulation is running, network statistics will be periodically printed in the console. You can interact with the simulation through the following keys:
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

- [Luigi Rapetta](https://github.com/rapfamily4)
- [Gianluca Torta](http://www.di.unito.it/~torta)
