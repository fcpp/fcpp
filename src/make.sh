#!/bin/bash

function usage() {
    echo -e "\033[4mcommands and parameters:\033[0m"
    echo -e "    \033[1mclean\033[0m:                           cleans all built files (can be chained)"
    echo -e "    \033[1mgcc\033[0m:                             sets the compiler to gcc (can be chained)"
    echo -e "    \033[1mdoc\033[0m:                             builds the documentation (can be chained)"
    echo -e "    \033[1mbuild\033[0m:                           builds binaries for given targets, skipping tests"
    echo -e "       <copts...> <targets...>"
    echo -e "    \033[1mtest\033[0m:                            builds binaries and tests for given targets"
    echo -e "       <copts...> <targets...>"
    echo -e "    \033[1mrun\033[0m:                             build and runs a single target"
    echo -e "       <copts...> <target> <arguments...>"
    echo -e "    \033[1mall\033[0m:                             builds all possible targets and documentation"
    echo -e "       <copts...>"
    echo -e "Targets can be substrings demanding builds for all possible expansions."
    exit 1
}

if [ "$1" == "" ]; then
    usage
fi

asan="--config=asan"
copts=""
targets=""

function mkdoc() {
    if [ ! -d doc ]; then
        mkdir doc
    fi
    doxygen Doxyfile
}

function parseopt() {
    i=0
    while [ "${1:0:1}" == "-" ]; do
        if [ "${1:0:2}" == "-O" ]; then
            asan="--config=opt"
        else
            copts="$copts $1"
        fi
        i=$[i+1]
        shift 1
    done
    if [ "$copts" != "" ]; then
        copts="--copt=\"${copts:1}\""
    fi
    return $i
}

function finder() {
    if [ "$targets" != "" ]; then
        return 0
    fi
    base="$1"
    find="$2"
    # exact match
    targets=`echo $base*/$find/ $base*/$find.cpp $base/*/$find.cpp | tr ' ' '\n' | grep -v "*"`
    # prefix match
    if [ "$targets" == "" ]; then
        targets=`echo $base/$find*/ $base/$find*.cpp $base/*/$find*.cpp | tr ' ' '\n' | grep -v "*"`
    fi
    # substring match
    if [ "$targets" == "" ]; then
        targets=`echo $base/*$find*/ $base/*$find*.cpp $base/*/*$find*.cpp | tr ' ' '\n' | grep -v "*"`
    fi
    # convert to bazel format
    if [ "$targets" != "" ]; then
        targets=`echo $targets | tr ' ' '\n' | rev | sed 's|^ppc.||;s|/|:|;s|^:|.../|' | rev | sort | uniq`
    fi
}

function builder() {
    cmd=$1
    shift 1
    for t in "$@"; do
        echo -e "\033[4mbazel $cmd $copts $asan //$t\033[0m"
        bazel $cmd $copts $asan //$t
    done
}

function runner() {
    t=$1
    if [ `echo $t | tr ' ' '\n' | wc -l` -ne 1 ]; then
        echo -e "\033[1mtarget is not unique\033[0m"
        echo $t | tr ' ' '\n' | sed 's|^|//|'
        exit 1
    fi
    shift 1
    echo -e "\033[4mbazel run $copts $asan -- //$t "$@"\033[0m"
    bazel run $copts $asan -- //$t "$@"
}

while [ "$1" != "" ]; do
    if [ "$1" == "doc" ]; then
        shift 1
        mkdoc
    elif [ "$1" == "build" ]; then
        shift 1
        parseopt "$@"
        shift $?
        if [ "$1" == "all" ]; then
            if [ "$2" != "" ]; then
                usage
            fi
            builder build lib/...
            builder build project/...
        else
            while [ "$1" != "" ]; do
                finder "project" "$1"
                finder "lib"    "$1"
                finder "test"   "$1"
                if [ "$targets" == "" ]; then
                    echo -e "\033[1mtarget \"$1\" not found\033[0m"
                fi
                builder build $targets
                targets=""
                shift 1
            done
        fi
        exit 0
    elif [ "$1" == "test" ]; then
        shift 1
        parseopt "$@"
        shift $?
        if [ "$1" == "all" ]; then
            if [ "$2" != "" ]; then
                usage
            fi
            builder test test/...
        else
            while [ "$1" != "" ]; do
                finder "test" "$1"
                finder "project" "$1"
                builder test  $targets
                if [ "$targets" == "" ]; then
                    echo -e "\033[1mtarget \"$1\" not found\033[0m"
                fi
                targets=""
                shift 1
            done
        fi
        exit 0
    elif [ "$1" == "run" ]; then
        shift 1
        parseopt "$@"
        shift $?
        finder "project" "$1"
        shift 1
        runner "$targets" "$@"
        exit 0
    elif [ "$1" == "all" ]; then
        shift 1
        parseopt "$@"
        shift $?
        if [ "$1" != "" ]; then
            usage
        fi
        mkdoc
        builder build lib/...
        builder build project/...
        builder build test/...
        builder test  test/...
        builder test  project/...
    elif [ "$1" == "clean" ]; then
        shift 1
        bazel clean
        rm -rf doc
    elif [ "$1" == "gcc" ]; then
        shift 1
        gcc=$(which $(compgen -c | grep "^gcc-.$" | uniq))
        gpp=$(which $(compgen -c | grep "^g++-.$" | uniq))
        export BAZEL_USE_CPP_ONLY_TOOLCHAIN=1
        export CC="$gcc"
        export CXX="$gpp"
    else
        usage
    fi
done
