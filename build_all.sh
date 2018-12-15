#!/bin/bash

if [ ! -d doc ]; then
    mkdir doc
fi
doxygen Doxyfile
bazel clean
bazel build --config=asan --incompatible_remove_native_http_archive=false //lib/...
bazel build --config=asan --incompatible_remove_native_http_archive=false //test/...
bazel test  --config=asan --incompatible_remove_native_http_archive=false //test/...
