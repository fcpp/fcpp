#!/bin/bash

function usage() {
    echo -e "\033[4mcommands and parameters:\033[0m"
    echo -e "    \033[1mclean\033[0m:                           cleans all built files (can be chained)"
    echo -e "    \033[1mhere\033[0m:                            sets the working directory here (can be chained)"
    echo -e "    \033[1mgcc\033[0m:                             sets the compiler to gcc (can be chained)"
    echo -e "    \033[1msed\033[0m:                             manipulates patterns in source files (can be chained)"
    echo -e "       <pattern> [replace]"
    echo -e "    \033[1mdoc\033[0m:                             builds the documentation (can be chained)"
    echo -e "    \033[1mbuild\033[0m:                           builds binaries for given targets, skipping tests"
    echo -e "       <copts...> <targets...>"
    echo -e "    \033[1mtest\033[0m:                            builds binaries and tests for given targets"
    echo -e "       <copts...> <targets...>"
    echo -e "    \033[1mrun\033[0m:                             build and runs a single target"
    echo -e "       <copts...> <target> <plots...> <arguments...>"
    echo -e "    \033[1mall\033[0m:                             builds all possible targets and documentation"
    echo -e "       <copts...>"
    echo -e "Targets can be substrings demanding builds for all possible expansions. Syntax for plots is:"
    plotter/plot_builder.py | tail -n +6 | head -n 10
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
folders=( `ls */BUILD | sed 's|/BUILD||'` )

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

function filter() {
    rule="$1"
    shift 1
    while read -r target; do
        build=`echo $target | sed 's|/[^/]*.cpp$|/BUILD|'`
        name=`basename $target .cpp`
        if [ -f $build -a `cat $build | tr -s ' \n' ' ' | grep "$rule( name = \"$name\"" | wc -l` -gt 0 ]; then
            echo -n "$target "
        fi
    done
    echo
}

function finder() {
    if [ "$targets" != "" ]; then
        return 0
    fi
    find="$1"
    rule="$2"
    for pattern in "$find" "$find*" "*$find*"; do
        if [ "$targets" == "" ]; then
            targets=$(for base in ${folders[@]}; do
                echo $base*/$pattern/ $base*/$pattern.cpp $base/*/$pattern.cpp | tr ' ' '\n' | grep -v "*" | filter "$rule"
            done)
        fi
    done
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
    elif [ "$1" == "sed" ]; then
        pattern="$2"
        replace=""
        videoreplace=`echo -e "\033[7m&\033[0m"`
        shift 2
        if [ "$1" != "" -a `echo $1 | grep "clean\|here\|gcc\|sed\|doc\|build\|test\|run\|all" | wc -l` -eq 0 ]; then
            replace="$1"
            videoreplace=`echo -e "\033[31m[-&-]\033[32m{+$replace+}\033[0m"`
            shift 1
        fi
        for folder in ${folders[@]}; do
            for f in $folder/*.?pp $folder/*/*.?pp; do
                if [ -f "$f" -a `cat "$f" | grep -E "$pattern" | wc -l` -gt 0 ]; then
                    echo -e "\n==> $f <=="
                    cat -n $f | grep -E "$pattern" | sed -E "s/$pattern/$videoreplace/g"
                fi
            done
        done | less -r
        totn=0
        totf=0
        for folder in ${folders[@]}; do
            for f in $folder/*.?pp $folder/*/*.?pp; do
                if [ -f "$f" -a `cat "$f" | grep -E "$pattern" | wc -l` -gt 0 ]; then
                    n=`cat $f | sed -E "s/$pattern/ß/g" | tr -cd "ß" | tr "ß" "x" | wc -c`
                    totn=$[totn+n]
                    totf=$[totf+1]
                fi
            done
        done
        echo "$totn occurrences found across $totf files."
        if [ "$replace" != "" ]; then
            echo -n "Proceed with substitution? (y/N) "
            read x
            if [ "$x" == "y" ]; then
                for folder in ${folders[@]}; do
                    for f in $folder/*.?pp $folder/*/*.?pp; do
                        if [ -f "$f" -a `cat "$f" | grep -E "$pattern" | wc -l` -gt 0 ]; then
                            sed -i "" -E "s/$pattern/$replace/g" $f
                        fi
                    done
                done
            fi
        fi
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
                for folder in ${folders[@]}; do
                    alltargets="$alltargets $folder/..."
                done
           else
                finder "$1" "\(cc_library\|cc_binary\)"
                finder "$1" "cc_test"
                if [ "$targets" == "" ]; then
                    echo -e "\033[1mtarget \"$1\" not found\033[0m"
                else
                    alltargets="$alltargets $targets"
                    targets=""
                fi
            fi
            shift 1
        done
        if [ "$alltargets" != "" ]; then
            builder build $alltargets
        fi
        quitter
    elif [ "$1" == "test" ]; then
        shift 1
        parseopt "$@"
        shift $?
        alltargets=""
        while [ "$1" != "" ]; do
            if [ "$1" == "all" ]; then
                for folder in ${folders[@]}; do
                    alltargets="$alltargets $folder/..."
                done
            else
                finder "$1" "cc_test"
                if [ "$targets" == "" ]; then
                    echo -e "\033[1mtarget \"$1\" not found\033[0m"
                else
                    alltargets="$alltargets $targets"
                    targets=""
                fi
            fi
            shift 1
        done
        if [ "$alltargets" != "" ]; then
            builder test $alltargets
        fi
        quitter
    elif [ "$1" == "run" ]; then
        shift 1
        parseopt "$@"
        shift $?
        finder "$1" "cc_binary"
        if [ "$targets" == "" ]; then
            echo -e "\033[1mtarget \"$1\" not found\033[0m"
        elif [ `echo $targets | tr ' ' '\n' | wc -l` -ne 1 ]; then
            echo -e "\033[1mtarget is not unique\033[0m"
            echo $targets | tr ' ' '\n' | sed 's|^|//|'
        else
            shift 1
            plots=( )
            while [ `echo "$1" | grep '(' | wc -l` -gt 0 ]; do
                plots=( "${plots[@]}" "$1" )
                shift 1
            done
            mkdir -p output/raw
            name=`echo $targets | sed 's|.*:||'`
            file="output/raw/$name.txt"
            echo -e "\033[4mbazel run $copts $asan -- //$targets "$@"\033[0m"
            reporter bazel run $copts $asan -- //$targets "$@" | tee $file
            if [ `cat $file | wc -c` -eq 0 ]; then
                rm $file
            fi
            if [ "${#plots[@]}" -gt 0 ]; then
                v=`ls output/$name-*.asy | sed "s|^output/$name-||;s|.asy$||" | sort -n | tail -n 1`
                v=$[v+1]
                plotter/plot_builder.py "output/raw/$name*.txt" "${plots[@]}" > output/$name-$v.asy
                cp plotter/plot.asy output/
                cd output
                asy $name-$v.asy -f pdf
                rm plot.asy
                cd ..
            fi
        fi
        quitter
    elif [ "$1" == "all" ]; then
        shift 1
        parseopt "$@"
        shift $?
        if [ "$1" != "" ]; then
            usage
        fi
        mkdoc
        for folder in ${folders[@]}; do
            alltargets="$alltargets $folder/..."
        done
        builder build $alltargets
        builder test $alltargets
        quitter
    else
        usage
    fi
done
