# FCPP - FieldCalc++

An efficient C++14 implementation of the Pure Field Calculus, for fast and effective simulation of pervasive computing scenarios, and deployment on microcontroller architectures.
Documentation of the API is available [here](http://fcpp-doc.surge.sh).

#### Status Badges

#### Stable branch
[![Build Status](https://travis-ci.com/fcpp/fcpp.svg?branch=master)](https://travis-ci.com/fcpp/fcpp)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/90634407d674499cb62da7d7d74e8b42)](https://app.codacy.com/gh/fcpp/fcpp?utm_source=github.com&utm_medium=referral&utm_content=fcpp/fcpp&utm_campaign=Badge_Grade_Dashboard)

#### Development branch
[![Build Status](https://travis-ci.com/fcpp/fcpp.svg?branch=dev)](https://travis-ci.com/fcpp/fcpp/branches)

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
- [Doxygen](http://www.doxygen.nl)

In the `src` folder, you should be able to run `./make.sh all`, getting output about building and testing the whole library.

## Contribute

### Authors

- [Giorgio Audrito](http://giorgio.audrito.info/#!/research)
