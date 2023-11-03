#!/bin/bash

###########################################################
###########################################################
RM="rm -rfd"
RED='\033[0;31m'
NC='\033[0m'
GREEN='\033[0;32m'
CURRENT_DIR_NAME=$(basename $(pwd))

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
    cd -
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

function send_code_to_barbora() {
    # This function is used to send code to Barbora

    # Create archive
    zip_name="xlapes02.zip"

    # Create archive
    git archive -o ${zip_name} HEAD

    # Send archive
    scp ${zip_name} 'avs_barbora:~/repos'

    # rm archive on local machine and on server
    rm ${zip_name}
    ssh avs_barbora "cd ~/repos && rm -rfd ${CURRENT_DIR_NAME} && unzip -d ${CURRENT_DIR_NAME} ${zip_name} && rm ${zip_name}"
}

function test_on_barbora() {
    # This function is used to test the code on Barbora
    # This copy code to Barbora, build it and run sbatch advisor.sl and evaluate.sl
    send_code_to_barbora
    # Run advisor and evaluate
    ssh avs_barbora "cd ~/repos/${CURRENT_DIR_NAME} && sbatch advisor.sl; sbatch evaluate.sl"
}

function backup() {
    CMD="squeue --me -l -t RUNNING --noheader | wc -l"
    CMD2="squeue --me -l -t PENDING --noheader | wc -l"
    while [ 1 ]; do
        if [ "$(ssh avs_barbora "$CMD")" == "0" ] && [ "$(ssh avs_barbora "$CMD2")" == "0" ]; then
            echo -e "${GREEN}All jobs finished${NC}"
            time=$(date +%Y-%m-%d_%H-%M)
            for i in "build_evaluate" "build_advisor"; do
                mkdir -p tmp/backups/${time}/${i}
            done
            rsync -avz -e ssh "avs_barbora:\$(pwd)/repos/${CURRENT_DIR_NAME}/build_evaluate/tmp_*" tmp/backups/${time}/build_evaluate/
            rsync -avz -e ssh "avs_barbora:\$(pwd)/repos/${CURRENT_DIR_NAME}/build_advisor/Advisor-*" tmp/backups/${time}/build_advisor/
            for i in "csv" "out"; do
                rsync -avz -e ssh "avs_barbora:\$(pwd)/repos/${CURRENT_DIR_NAME}/*.${i}" tmp/backups/${time}/
            done
            break
        else
            echo -e "${RED}Some jobs are still running: Waiting 10s${NC}"
            sleep 10
        fi
    done
}

function watch_queue() {
    # Watch queue
    watch -n 1 "squeue -l | awk 'BEGIN{sum=0; psum = 0} {if(\$5 == \"RUNNING\") sum +=\$8; if (\$5 == \"PENDING\" && \$2 == \"qcpu_exp\") psum +=1;} END {printf \"%+4s/201 nodes used at the moment\n\", sum; printf \"%+4s nodes free\n\", 201-sum; printf \"%+4s pending jobs in queue qcpu_exp\n\", psum}'"
}

function usage() {
    # Print usage on stdout
    help
}

function check_mem_leaks() {
    DEBUG=1 build
#    valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=tmp/valgrind.out ./build/mandelbrot -c batch -s 512 tmp/res_batch.npz
    valgrind ./build/mandelbrot -c batch -s 512 tmp/res_batch.npz || true
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
