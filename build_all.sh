#!/bin/bash

if [ ! -d doc ]; then
    mkdir doc
fi

if [ "$1" == "doc" ]; then
    doxygen Doxyfile
elif [ "$1" == "full" ]; then
    doxygen Doxyfile
    bazel clean
    bazel build --config=asan --incompatible_remove_native_http_archive=false //lib/...
    bazel build --config=asan --incompatible_remove_native_http_archive=false //test/...
    bazel test  --config=asan --incompatible_remove_native_http_archive=false //test/...
elif [ "$1" != "" ]; then
    target=`echo test/*/"$1"* | sed -E 's|/([^/]*)\.cpp|:\1|'`
    if [ "$target" == "test/*/$1*" ]; then
        target=`echo test/"$1"* "/..." | tr -d ' '`
    fi
    bazel test  --config=asan --incompatible_remove_native_http_archive=false //"$target"
else
    bazel build --config=asan --incompatible_remove_native_http_archive=false //lib/...
    bazel build --config=asan --incompatible_remove_native_http_archive=false //test/...
    bazel test  --config=asan --incompatible_remove_native_http_archive=false //test/...
fi
