# FCPP - Field Calculus++

FCPP is a C++14 implementation of the Pure Field Calculus, for fast and effective simulation of pervasive computing scenarios.
Documentation of the API is available [here](brokenlink).

#### Status Badges

[![Build Status](https://travis-ci.org/Harniver/fcpp.svg?branch=master)](https://travis-ci.org/Harniver/fcpp)

## Getting Started

### Virtual Environment

In order to get started in a virtual environment, you should install [Vagrant](https://www.vagrantup.com). Then from the `src` folder you can do:
```
vagrant up
vagrant ssh
cd fcpp/src
./make.sh all
```
Then you should get output about building and testing the whole library (in the Vagrant virtual machine). After that you can exit the virtual machine through:
```
exit
vagrant halt
```
### Custom Build

In order to get started on your machine you need the following installed:

- [Bazel](https://bazel.build) (tested with versions 2.1.0+)
- [GCC](https://gcc.gnu.org) (tested with versions 9.2.0+)

In the `src` folder, you should be able to run `./make.sh all`, getting output about building and testing the whole library.

## Contribute

### Authors

- [Giorgio Audrito](http://giorgio.audrito.info/#!/research)
