#!/bin/bash

if [ ! -d doc ]; then
    mkdir doc
fi
doxygen Doxyfile
bazel clean
bazel build //lib/...
bazel build //test/...
bazel test  //test/...
