#!/bin/bash

if [ -z "$1" ]; then
  echo "No target provided. Using default target"
  set -- "Write rigister wordks"
fi
# Check if the target parameter is provided
# if [ -z "$1" ]; then
#   echo "Usage: $0 <target>"
#   exit 1
# fi

SCRIPT_DIR=$(dirname "$0")
cd ${SCRIPT_DIR}/../build/test

# Run the test with the provided target
./tests "$1"
echo $PWD

