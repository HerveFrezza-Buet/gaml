#!/bin/bash

# To list the examples :
# cd gaml-mlp/travis
# ls -l ../build/examples/example-* | awk '{print $NF}'

../build/examples/example-001-basics
../build/examples/example-002-train-ukf
../build/examples/example-003-real-risk
# long...
# ../build/examples/example-004-ukf-classification
../build/examples/example-005-test-gradient 0
../build/examples/example-005-test-gradient 1
# long...
# ../build/examples/example-005-train-gradient
../build/examples/example-006-gradient-classification

