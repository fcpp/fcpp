#!/bin/bash

function usage() {
    echo -e "\033[4musage:\033[0m"
    echo -e "    \033[1m./make.sh [options...] command [parameters...]\033[0m"
    echo
    echo -e "\033[4moptions:\033[0m"
    echo -e "    \033[1mclean\033[0m:                           cleans all built files before command execution"
    echo -e "    \033[1mdoc\033[0m:                             builds the documentation before command execution"
    echo -e "    \033[1mgui\033[0m:                             enables the graphical user interface on cmake"
    echo -e "    \033[1munix\033[0m:                            overrides the auto-detected cmake platform to unix"
    echo -e "    \033[1mwindows\033[0m:                         overrides the auto-detected cmake platform to windows"
    echo -e "    \033[1mbazel\033[0m:                           sets the build tool to bazel instead of cmake"
    echo -e "    \033[1mhere\033[0m:                            sets the bazel working directory here"
    echo -e "    \033[1mgcc\033[0m:                             sets the bazel compiler to gcc"
    echo
    echo -e "\033[4mcommands and parameters:\033[0m"
    echo -e "    \033[1msed\033[0m:                             manipulates patterns in source files"
    echo -e "       <pattern> [replace]"
    echo -e "    \033[1mbuild\033[0m:                           builds binaries for given targets"
    echo -e "       <copts...> <targets...>"
    echo -e "    \033[1mtest\033[0m:                            builds and run tests for given targets"
    echo -e "       <copts...> <targets...>"
    echo -e "    \033[1mrun\033[0m:                             build and runs given targets"
    echo -e "       <copts...> <targets...>"
    echo -e "    \033[1mall\033[0m:                             builds all possible targets and documentation"
    echo -e "       <copts...>"
    echo -e "Targets can be substrings demanding builds for all possible expansions."
    exit 1
}

if [ "$1" == "" ]; then
    usage
fi

btype="Debug"
opts=""
copts=""
cmakeopts=""
targets=""
errored=( )
exitcodes=( )
builder=cmake
if [ -f BUILD ]; then
    folders=( `ls */BUILD | sed 's|/BUILD||'` )
else
    folders=( `ls . | grep lib` `ls . | grep run` `ls . | grep test` )
fi
if [ "$OSTYPE" == "msys" ]; then
    platform="MinGW"
else
    platform="Unix"
fi

function numformat {
    n=$1
    k=$2
    if [ "$n" == "?" ]; then
	l=$[k-1]
    else
        n=$[n+0]
        l=${#n}
        l=$[k-l]
    fi
    for ((x=0; x<l; ++x)); do
        n=" $n"
    done
    echo -n "$n"
}

function ramformat {
    numformat "$1" 4
    echo -n " MB"
}

function addzero {
    n=$1
    if [ $n -lt 10 ]; then
            n=0$n
    fi
    echo -n $n
}

function timeformat {
    if [[ "$1" =~ [0-9][0-9]:[0-9][0-9]:[0-9][0-9] ]]; then
        echo -n "$1".00
    else
        mins=`echo $1 | sed 's|:.*||'`
        hour=$[mins/60]
        mins=$[mins%60]
        secs=`echo $1 | sed 's|^[0-9]*:||'`
        echo -n `addzero $hour`:`addzero $mins`:$secs
    fi
}

function reporter() {
    echo -en "\033[4m" >&2
    spacing=""
    for p in "$@"; do
        if [ `echo $p | wc -w` -gt 1 ]; then
            p="\"$p\""
        fi
        echo -n "$spacing$p" >&2
        spacing=" "
    done
    echo -e "\033[0m" >&2
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
    echo -e "\033[4mdoxygen Doxyfile\033[0m" >&2
    doxygen Doxyfile 2>&1 | grep -v "Generating docs\|\.\.\.\|Searching for" | tee tmpdoc.err | grep -v "is not documented"
    ndoc=`cat tmpdoc.err | grep "is not documented" | wc -l | tr -cd '0-9'`
    if [ $ndoc -gt 0 ]; then
        echo -e "\033[1m$ndoc items are not documented:\033[0m" >&2
        cat tmpdoc.err | grep "is not documented"
    fi
    rm tmpdoc.err
}

function parseopt() {
    i=0
    while [ "${1:0:1}" == "-" ]; do
        if [ "${1:0:2}" == "-O" ]; then
            btype="Release"
        else
            copts="$copts --copt=$1"
            cmakeopts="$cmakeopts $1"
        fi
        i=$[i+1]
        shift 1
    done
    if [ "$cmakeopts" != "" ]; then
        cmakeopts="-DCMAKE_CXX_FLAGS=${cmakeopts:1}"
    fi
    return $i
}

function filter() {
    rule="$1"
    shift 1
    while read -r target; do
        build=`echo $target | sed 's|/[^/]*.cpp$|/|'`BUILD
        if [ "${target: -4}" == ".cpp" ]; then
            name="['\"]`basename $target .cpp`['\"]"
        else
            name=""
        fi
        if [ -f $build -a `cat $build | tr -s ' \r\n' ' ' | grep "$rule( name = $name" | wc -l` -gt 0 ]; then
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

function cmake_finder() {
    search_folder="$1"
    search_sources="$2"
    search_suffix="$3"
    shift 3
    alltargets=""
    for find in "$@"; do
        targets=""
        if [ "$find" == "all" ]; then
            targets=$(echo $search_folder/*)
        fi
        for pattern in "$find" "$find*" "*$find*"; do
            if [ "$targets" == "" ]; then
                targets=$(echo $search_folder*/$pattern$search_suffix | tr ' ' '\n' | grep -v "*")
            fi
            if [ "$targets" == "" ]; then
                targets=$(echo $search_sources/$pattern/*.cpp | tr ' ' '\n' | grep -v "*" | sed "s|.cpp$|$search_suffix|;s|^$search_sources/[^/]*/|$search_folder/|")
            fi
        done
        if [ "$targets" == "" ]; then
            echo -e "\033[1mtarget \"$find\" not found\033[0m" >&2
        else
            alltargets="$alltargets $targets"
        fi
    done
    echo $alltargets | tr -s ' ' '\n' | grep . | sort | uniq
}

function builder() {
    cmd=$1
    shift 1
    t=`echo " $@" | sed 's| | //|g'`
    if [ "$btype" == "Debug" ]; then
        asan="--features=asan"
    elif [ "$btype" == "Release" ]; then
        asan="--features=opt"
    fi
    reporter bazel $cmd $copts $asan $t
}

function cmake_builder() {
    parseopt "$@"
    nshift=$?
    if [ hash nproc 2>/dev/null ]; then
        opt="-j `nproc`"
    fi
    reporter cmake -S ./ -B ./bin -G "$platform Makefiles" -DCMAKE_BUILD_TYPE=$btype $opts "$cmakeopts"
    reporter cmake --build ./bin/ $opt
    return $nshift
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
        rm -rf doc bin
        if [ $builder == bazel ]; then
            bazel clean
        fi
    elif [ "$1" == "here" ]; then
        shift 1
        export TEST_TMPDIR=`pwd`/..
    elif [ "$1" == "gcc" ]; then
        shift 1
        gcc=$(which $(compgen -c gcc- | grep "^gcc-[1-9][0-9]*$" | uniq))
        gpp=$(which $(compgen -c g++- | grep "^g++-[1-9][0-9]*$" | uniq))
        export BAZEL_USE_CPP_ONLY_TOOLCHAIN=1
        export CC="$gpp"
        export CXX="$gpp"
    elif [ "$1" == "doc" ]; then
        shift 1
        mkdoc
    elif [ "$1" == "gui" ]; then
        shift 1
        opts="$opts -DFCPP_BUILD_GL=ON"
    elif [ "$1" == "windows" ]; then
        shift 1
        platform="MinGW"
    elif [ "$1" == "unix" ]; then
        shift 1
        platform="Unix"
    elif [ "$1" == "bazel" ]; then
        shift 1
        builder="bazel"
    elif [ "$1" == "sed" ]; then
        pattern="$2"
        replace=""
        videoreplace=`echo -e "\033[7m&\033[0m"`
        replacing=0
        shift 2
        if [ "$1" != "" -a `echo $1 | grep "clean\|here\|gcc\|sed\|doc\|build\|test\|run\|all" | wc -l` -eq 0 ]; then
            replace="$1"
            if [ "$replace" == "del" ]; then
                replace=""
            fi
            replacing=1
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
        if [ $replacing -eq 1 ]; then
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
        quitter
    elif [ "$1" == "build" ]; then
        shift 1
        if [ $builder == cmake ]; then
            cmake_builder "$@"
            shift $?
        else
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
        fi
        quitter
    elif [ "$1" == "run" ]; then
        shift 1
        function monitor() {
            pid=$1
            name=$2
            file=$3
            raw=$4
            trap ctrl_c INT
            function ctrl_c() {
                echo -e "\n\033[J"
                kill -9 $pid 2>&1
                exit 1
            }
            echo -e "\033[4mRUNNING: CPU TIME     RAM (NOW)   (AVG)   (MAX)   FILES   LINES\033[0m"
            num=0
            max=0
            sum=0
            while true; do
                tim=`ps -o time -p $pid | tail -n +2 | tr -d ' \t\n'`
                m=`ps -o rss -p $pid | tail -n +2 | tr -d ' \t\n'`
                if [ "$m" == "" ]; then break; else mem=$m; fi
                num=$[num+1]
                sum=$[sum+mem]
                mem=$[(mem+511)/1024]
                max=$[max > mem ? max : mem]
                avg=$[((sum+511)/1024 + num/2)/ num]
                fil=`ls $raw | grep $"$name.*\.txt" | wc -l`
                if [ "$fil" -gt 1000 ]; then
                    row="?"
                elif [ "$fil" -eq 0 ]; then
                    row=0
                else
                    row=`cat $raw/$name*.txt | grep -v "^#" | wc -l`
                fi
                echo -e "         `timeformat $tim`s   `ramformat $mem` `ramformat $avg` `ramformat $max` `numformat $fil 7` `numformat $row 7`\n\033[J"
                ( cat $file.txt | tail -n 10 | cut -c 1-`tput cols`; echo -e "\n\n\n\n\n\n\n\n\n" ) | head -n 10
                if [ `cat $file.err | wc -c` -eq 0 ]; then
                    echo -e "\n"
                else
                    echo -e "\n\033[4mSTDERR:\033[0m"
                fi
                ( cat $file.err | tail -n  5 | cut -c 1-`tput cols`; echo -e "\n\n\n\n\n\n\n\n\n" ) | head -n 5
                echo -en "\033[19A"
                sleep 1
            done
            echo -e "\n\033[J"
            if [ `cat $file.txt | grep 'import "plot.asy" as plot;' | wc -l` -eq 1 ]; then
                asy=`basename $file`
                cp $file.txt plot/$asy.asy
                cd plot
                asy $asy.asy -f pdf
                cd ..
            fi
            if [ `cat $file.txt | wc -c` -eq 0 ]; then
                rm $file.txt
            fi
            if [ `cat $file.err | wc -c` -eq 0 ]; then
                rm $file.err
            fi
            nnn=`echo $raw/${name}_*.txt`
            if [ "$nnn" != "$raw/${name}_*.txt" ]; then
                mv $raw/${name}_*.txt output/raw/
            fi
        }
        if [ $builder == cmake ]; then
            cmake_builder "$@"
            shift $?
            for target in $(cmake_finder bin/run run "" "$@"); do
                if [ ${#exitcodes[@]} -gt 0 ]; then
                    quitter
                fi
                name="${target:8}"
                file="output/$name"
                raw="bin/output"
                mkdir -p bin/output output/raw
                cd bin
                echo -e "\033[1;4m$target\033[0m\n"
                run/$name > ../$file.txt 2> ../$file.err & pid=$!
                cd ..
                monitor $pid $name $file $raw
            done
        else
            parseopt "$@"
            shift $?
            finder "$1" "cc_binary"
            if [ "$targets" == "" ]; then
                echo -e "\033[1mtarget \"$1\" not found\033[0m"
            elif [ `echo $targets | tr ' ' '\n' | wc -l` -ne 1 ]; then
                echo -e "\033[1mtarget is not unique\033[0m"
                echo $targets | tr ' ' '\n' | sed 's|^|//|'
            else
                name=`echo $targets | sed 's|.*:||'`
                file="output/$name"
                raw="output"
                built=`echo bazel-bin/$targets | tr ':' '/'`
                builder build $targets
                if [ ${#exitcodes[@]} -gt 0 ]; then
                    quitter
                fi
                mkdir -p output/raw
                $built > $file.txt 2> $file.err & pid=$!
                monitor $pid $name $file $raw
            fi
        fi
        quitter
    elif [ "$1" == "test" ]; then
        shift 1
        if [ $builder == cmake ]; then
            opts="$opts -DFCPP_BUILD_TESTS=ON"
            cmake_builder "$@"
            shift $?
            for target in $(cmake_finder bin/test test _test "$@"); do
                reporter $target
            done
        else
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
        fi
        quitter
    elif [ "$1" == "all" ]; then
        shift 1
        mkdoc
        if [ $builder == cmake ]; then
            opts="$opts -DFCPP_BUILD_TESTS=ON"
            cmake_builder "$@"
        else
            parseopt "$@"
        fi
        shift $?
        if [ "$1" != "" ]; then
            usage
        fi
        if [ $builder == cmake ]; then
            for target in $(cmake_finder bin/test test _test all); do
                reporter $target
            done
        else
            for folder in ${folders[@]}; do
                alltargets="$alltargets $folder/..."
            done
            builder build $alltargets
            builder test $alltargets
        fi
        quitter
    else
        usage
    fi
done
