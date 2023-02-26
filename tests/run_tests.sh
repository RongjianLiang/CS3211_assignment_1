#!/bin/bash

shopt -s nullglob globstar
arr=(**/*.in)
IFS=$'\n'
pushd .
cd ..
make
popd
if [[ "$1" == "all" ]]; then
  for filename in ${arr[*]}; do
    BASE_FILENAME=$(basename "$filename" .in)
    CONTAINING_DIRECTORY=$(dirname "$filename")
    # read -p "Run $filename? (Y/N): " confirm
    #if [[ $confirm == [yY] || $confirm == [yY][eE][sS] ]]; then
    mkdir -p "$BASE_FILENAME"
    cd "./${BASE_FILENAME}"
    ../../grader-linux ../../engine < "../${filename}" > "${BASE_FILENAME}.log" 2>&1
  
    TAIL=$(tail -n 1 "${BASE_FILENAME}.log")
    if [[ $TAIL == "test passed" ]]; then
        echo "Passed $filename"
    else
        echo
        echo "***Failed to run $filename***"
    fi
    cd ..
    #fi
  done
else
  BASE_FILENAME=$(basename "$1" .in)
  echo $BASE_FILENAME
  CONTAINING_DIRECTORY=$(dirname "$1")
  mkdir -p "./results/${BASE_FILENAME}"
  cd "./results/${BASE_FILENAME}"

  for i in {0..9}; do
    ../../../grader-linux ../../../engine < "../../$1" > "${BASE_FILENAME}_${i}.log" 2>&1
    TAIL=$(tail -n 1 "${BASE_FILENAME}_${i}.log")
    if [[ $TAIL == "test passed." ]]; then
        rm "${BASE_FILENAME}_${i}.log"
    else
        echo "***Failed to run $1*** for $i"
    fi
  done
  
  cd ../..
fi
unset IFS

