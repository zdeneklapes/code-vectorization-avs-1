#!/bin/bash

###########################################################
###########################################################
RM="rm -rfd"
RED='\033[0;31m'
NC='\033[0m'
GREEN='\033[0;32m'

function clean() {
    # Folders
    for folder in \
        "cmake-build-debug*" \
        "build_advisor" \
        "build"; do
        if [ "$DEBUG" -eq 1 ]; then find . -type d -iname "${folder}"; else find . -type d -iname "${folder}" | xargs ${RM} -rf; fi
    done

    # Files
    for file in \
        "*.DS_Store" \
        "tags" \
        "slurm*.out" \
        "*.zip"; do
#        "db.sqlite3" \
#        "*.png" \
#        "*.log" \
#        "coverage.xml" \
#        "*.coverage" \
#        "coverage.lcov"; do
        if [ "$DEBUG" -eq 1 ]; then find . -type f -iname "${file}"; else find . -type f -iname "${file}" | xargs ${RM}; fi
    done
}

function build() {
    mkdir -p "build"
    cd "build"
    echo $(pwd)
    if [ "$DEBUG" -eq 1 ]; then DEBUG="$DEBUG" cmake ..; else CC=icc CXX=icpc cmake ..; fi
    make -j
}

function pack() {
    # Clean and Zip project
    ZIP_NAME="xlapes02.zip"

    CMD="zip -r '$ZIP_NAME' \
        BatchMandelCalculator.cc \
        BatchMandelCalculator.h \
        LineMandelCalculator.cc \
        LineMandelCalculator.h \
        eval.png \
        MB-xlogin.txt"

    if [ $DEBUG -eq 1 ]; then echo "$CMD"; else eval "$CMD"; fi
}

function test_fast() {
    # run clean
    DEBUG=0 clean
    DEBUG=0 build
    make -j -C build
    for i in "ref" "line" "batch"; do
        echo "Running test $i"
        ./build/mandelbrot -c ref -s 4096 res.npz
    done
    ./build/mandelbrot -c ref -s 4096 res.npz
}


function help() {
    # Print usage on stdout
    echo "Available functions:"
    for file in "make.sh"; do
        function_names=$(cat ${file} | grep -E "(\ *)function\ +.*\(\)\ *\{" | sed -E "s/\ *function\ +//" | sed -E "s/\ *\(\)\ *\{\ *//")
        for func_name in ${function_names[@]}; do
            printf "    $func_name\n"
            awk "/function ${func_name}()/ { flag = 1 }; flag && /^\ +#/ { print \"        \" \$0 }; flag && !/^\ +#/ && !/function ${func_name}()/  { print "\n"; exit }" ${file}
        done
    done
}

function rsync_to_barbora() {
    rsync -avz --exclude={"*.git", ".idea", "tmp", "cmake-build-debug*", "build*", "*.pdf", "*.zip"} -e ssh . "avs_barbora:\$(pwd)/repos/code-vectorization-avs-1"
}

function rsync_to_macbook() {
    time=$(date +%Y-%m-%d_%H-%M)

    for i in "build_evaluate" "build_advisor"; do
        mkdir -p tmp/backups/${time}/${i}
        rsync -avz -e ssh "avs_barbora:\$(pwd)/repos/code-vectorization-avs-1/${i}/**" tmp/backups/${time}/${i}
    done

    for i in "csv" "out"; do
        rsync -avz -e ssh "avs_barbora:\$(pwd)/repos/code-vectorization-avs-1/*.${i}" tmp/backups/${time}/
    done
}

function usage() {
    # Print usage on stdout
    help
}

function die() {
    # Print error message on stdout and exit
    printf "${RED}ERROR: $1${NC}\n"
    help
    exit 1
}

function main() {
    # Main function: Call other functions based on input arguments
    [[ "$#" -eq 0 ]] && die "No arguments provided"
    while [ "$#" -gt 0 ]; do
        "$1" || die "Unknown function: $1()"
        shift
    done
}

main "$@"
