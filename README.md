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
- [MinGW-w64 builds 8.1.0](http://mingw-w64.org/doku.php/download/mingw-builds)
- [CMake 3.9](https://cmake.org) (or higher)

During CMake installation, make sure you select to add `cmake` to the `PATH` (at least for the current user).
During MinGW installation, make sure you select "posix" threads (should be the default) and not "win32" threads. After installing MinGW, you need to add its path to the environment variable `PATH`. The default path should be:
```
C:\Program Files (x86)\mingw-w64\i686-8.1.0-posix-dwarf-rt_v6-rev0\mingw32\bin
```
but the actual path may vary depending on your installation. Then, you should be able to build the whole library with CMake through:
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

- [Gianmarco Rampulla](https://github.com/denoide1)
- [Luigi Rapetta](https://github.com/rapfamily4)
- [Gianluca Torta](http://www.di.unito.it/~torta)
