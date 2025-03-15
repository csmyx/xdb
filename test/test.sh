#!/bin/bash

# if [ -z "$1" ]; then
#   echo "No target provided. Using default target"
#   set -- "Write rigister works"
# fi
# Check if the target parameter is provided
# if [ -z "$1" ]; then
#   echo "Usage: $0 <target>"
#   exit 1
# fi

SCRIPT_DIR=$(dirname "$0")
cd ${SCRIPT_DIR}/../build/test
# echo $PWD

# Run the test with the provided target
if [ -n "$1" ]; then
  ./tests "$1"
else
  ./tests
fi
