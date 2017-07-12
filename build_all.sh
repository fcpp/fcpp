#!/bin/bash

if [ ! -d doc ]; then
    mkdir doc
fi
doxygen Doxyfile
bazel clean
bazel build --config=asan //lib/...
bazel build --config=asan //test/...
bazel test  --config=asan //test/...
