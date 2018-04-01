#!/bin/bash

# To list the examples :
# cd gaml-libsvm/travis
# ls -l ../build/examples/example-* | awk '{print $NF}'
../build/examples/example-001-basics 500
../build/examples/example-002-unsupervized 500
../build/examples/example-003-3D
../build/examples/example-004-grid-search

