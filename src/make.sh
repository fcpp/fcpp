#!/bin/bash

function usage() {
    echo -e "\033[4mcommands and parameters:\033[0m"
    echo -e "    \033[1mclean\033[0m:                           cleans all built files (can be chained)"
    echo -e "    \033[1mhere\033[0m:                            sets the working directory here (can be chained)"
    echo -e "    \033[1mgcc\033[0m:                             sets the compiler to gcc (can be chained)"
    echo -e "    \033[1mgrep\033[0m:                            searches for a pattern in the library"
    echo -e "       <pattern>"
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
errored=( )
exitcodes=( )
folders=( `ls */BUILD | sed 's|/BUILD||' | grep -v "^lib$" | grep -v "^test$"` )

function reporter() {
    "$@"
    code=$?
    if [ $code -gt 0 ]; then
        exitcodes=( ${exitcodes[@]} $code )
        failcmd="\033[4m$@\033[0m"
        errored=( "${errored[@]}" "$failcmd" )
    fi
    return ${#exitcodes[@]}
}

function quitter() {
    code=${#exitcodes[@]}
    if [ $code -gt 0 ]; then
        echo
        echo -e "\033[1mBuild terminated with errors:\033[0m"
        for ((i=0; i<code; ++i)); do
            echo -e "${errored[i]}: exit with ${exitcodes[i]}"
        done
    fi
    exit $code
}

function mkdoc() {
    if [ ! -d doc ]; then
        mkdir doc
    fi
    reporter doxygen Doxyfile
}

function parseopt() {
    i=0
    while [ "${1:0:1}" == "-" ]; do
        if [ "${1:0:2}" == "-O" ]; then
            asan="--config=opt"
        else
            copts="$copts --copt=$1"
        fi
        i=$[i+1]
        shift 1
    done
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
    t=`echo " $@" | sed 's| | //|g'`
    echo -e "\033[4mbazel $cmd $copts $asan $t\033[0m"
    reporter bazel $cmd $copts $asan $t
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
    reporter bazel run $copts $asan -- //$t "$@"
}

function powerset() {
    first="$1"
    if [ "$first" == "" ]; then
        echo ""
        exit 0
    fi
    shift 1
    rec=`powerset "$@"`
    echo -n "$rec"
    echo " $rec" | sed "s| | #$first|g"
}

while [ "$1" != "" ]; do
    if [ "$1" == "clean" ]; then
        shift 1
        rm -rf doc
        bazel clean
    elif [ "$1" == "here" ]; then
        shift 1
        export TEST_TMPDIR=`pwd`
    elif [ "$1" == "gcc" ]; then
        shift 1
        gcc=$(which $(compgen -c | grep "^gcc-.$" | uniq))
        gpp=$(which $(compgen -c | grep "^g++-.$" | uniq))
        export BAZEL_USE_CPP_ONLY_TOOLCHAIN=1
        export CC="$gcc"
        export CXX="$gpp"
    elif [ "$1" == "grep" ]; then
        pattern="$2"
        shift 2
        for folder in lib test ${folders[@]}; do
            for f in $folder/*.?pp $folder/*/*.?pp; do
                if [ `cat "$f" | grep "$pattern" | wc -l` -gt 0 ]; then
                    echo -e "\n==> $f <=="
                    cat -n $f | grep "$pattern"
                fi
            done
        done | less
    elif [ "$1" == "doc" ]; then
        shift 1
        mkdoc
    elif [ "$1" == "build" ]; then
        shift 1
        parseopt "$@"
        shift $?
        alltargets=""
        while [ "$1" != "" ]; do
            if [ "$1" == "all" ]; then
                alltargets="$alltargets lib/... test/..."
                 for folder in ${folders[@]}; do
                    alltargets="$alltargets $folder/..."
                done
           else
                for folder in ${folders[@]}; do
                    finder $folder "$1"
                done
                finder "lib"    "$1"
                finder "test"   "$1"
                alltargets="$alltargets $targets"
                if [ "$targets" == "" ]; then
                    echo -e "\033[1mtarget \"$1\" not found\033[0m"
                fi
                targets=""
            fi
            shift 1
        done
        builder build $alltargets
        quitter
    elif [ "$1" == "test" ]; then
        shift 1
        parseopt "$@"
        shift $?
        alltargets=""
        while [ "$1" != "" ]; do
            if [ "$1" == "all" ]; then
                alltargets="$alltargets test/..."
                for folder in ${folders[@]}; do
                    alltargets="$alltargets $folder/..."
                done
            else
                finder "test" "$1"
                for folder in ${folders[@]}; do
                    finder $folder "$1"
                done
                alltargets="$alltargets $targets"
                if [ "$targets" == "" ]; then
                    echo -e "\033[1mtarget \"$1\" not found\033[0m"
                fi
                targets=""
            fi
            shift 1
        done
        builder test $alltargets
        quitter
    elif [ "$1" == "run" ]; then
        shift 1
        parseopt "$@"
        shift $?
        for folder in ${folders[@]}; do
            finder $folder "$1"
        done
        shift 1
        runner "$targets" "$@"
        quitter
    elif [ "$1" == "all" ]; then
        shift 1
        parseopt "$@"
        shift $?
        if [ "$1" != "" ]; then
            usage
        fi
        mkdoc
        alltargets="test/..."
        for folder in ${folders[@]}; do
            alltargets="$alltargets $folder/..."
        done
        builder test $alltargets
        quitter
    else
        usage
    fi
done
