#!/usr/bin/bash

SCRIPT_ROOT_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
CALCULATORS=("ref" "line" "batch")

for calc in "${CALCULATORS[@]}"; do
    ./build/mandelbrot -s 512 -i 100 -c $calc --batch tmp/cmp_$calc.npz
done

for calc in "${CALCULATORS[@]}"; do
    python3 scripts/visualise.py --save tmp/cmp_$calc.png tmp/cmp_$calc.npz
done

VALID=1
echo "Reference vs line"
python3 ${SCRIPT_ROOT_PATH}/compare.py tmp/cmp_ref.npz tmp/cmp_line.npz  || VALID=0

echo "Reference vs batch"
python3 ${SCRIPT_ROOT_PATH}/compare.py tmp/cmp_ref.npz tmp/cmp_batch.npz || VALID=0

echo "Batch vs line"
python3 ${SCRIPT_ROOT_PATH}/compare.py tmp/cmp_line.npz tmp/cmp_batch.npz || VALID=0

if [ "$VALID" -eq 1 ]; then
    echo "Test passed";
else
    echo "Test failed";
    exit 1
fi


