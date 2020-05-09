# FCPP - FieldCalc++

An efficient C++14 implementation of the Pure Field Calculus, for fast and effective simulation of pervasive computing scenarios, and deployment on microcontroller architectures.

## Users: Getting Started

Start from the [main website](https://fcpp.github.io) of this project, while keeping the [documentation of the API](http://fcpp-doc.surge.sh) handy. The repository is set up to be run in a virtual environment through [Vagrant](https://www.vagrantup.com) or [Docker](https://www.docker.com/). Alternatively, you can directly run it in your local machine through a custom build.

### Vagrant

Type the following commands in a terminal, starting from the `src` folder of the repository:
```
vagrant up
vagrant ssh
cd fcpp
./make.sh all
```
Then you should get output about building and testing the whole library (in the Vagrant virtual machine). After that you can exit and stop the virtual machine through:
```
exit
vagrant halt
```

### Docker

Type the following commands in a terminal, starting from the `src` folder of the repository:
```
docker build -t fcpp .
docker run -it --volume $PWD:/fcpp --workdir /fcpp fcpp
./make.sh all
```
Then you should get output about building and testing the whole library (in the Docker container). After that you can exit and stop the container through:
```
exit
```

### Custom Build

In order to get started on your machine you need the following installed:

- [Bazel](https://bazel.build) (tested with version 2.1.0)
- [GCC](https://gcc.gnu.org) (tested with version 9.2.0)
- [Doxygen](http://www.doxygen.nl)

In the `src` folder, you should be able to run `./make.sh all`, getting output about building and testing the whole library.

## Developers: Contribute

### Status Badges

#### Stable Branch
[![Build Status](https://travis-ci.com/fcpp/fcpp.svg?branch=master)](https://travis-ci.com/fcpp/fcpp)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/90634407d674499cb62da7d7d74e8b42)](https://app.codacy.com/gh/fcpp/fcpp?utm_source=github.com&utm_medium=referral&utm_content=fcpp/fcpp&utm_campaign=Badge_Grade_Dashboard)

#### Development Branch
[![Build Status](https://travis-ci.com/fcpp/fcpp.svg?branch=dev)](https://travis-ci.com/fcpp/fcpp/branches)

### Authors

- [Giorgio Audrito](http://giorgio.audrito.info/#!/research)
