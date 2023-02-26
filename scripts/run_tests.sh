#!/bin/bash

shopt -s nullglob globstar
arr=(**/*.in)
IFS=$'\n'
pushd .
cd ..
make
popd

run_individual_test () {
  BASE_FILENAME=$(basename "$1" .in)
  CONTAINING_DIRECTORY=$(dirname "$1")
  mkdir -p "./results/${BASE_FILENAME}"
  cd "./results/${BASE_FILENAME}"

  for i in {0..1}; do
    ../../../grader-linux ../../../engine < "../../$1" > "${BASE_FILENAME}_${i}.log" 2>&1
    TAIL=$(tail -n 1 "${BASE_FILENAME}_${i}.log")
    if [[ $TAIL == "test passed." ]]; then
        rm "${BASE_FILENAME}_${i}.log"
    else
        echo "***Failed to run $1*** for $i"
    fi
  done
  cd ../..
}

    # read -p "Run $filename? (Y/N): " confirm
    #if [[ $confirm == [yY] || $confirm == [yY][eE][sS] ]]; then


if [[ "$1" == "all" ]]; then
  for filename in ${arr[*]}; do
    run_individual_test $filename
  done
else
  run_individual_test $1
fi

find ./ -empty -type d -delete
unset IFS
