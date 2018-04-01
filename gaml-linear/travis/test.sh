#!/bin/bash

# To list the examples :
# cd gaml-linear/travis
# ls -l ../build/examples/example-* | awk '{print $NF}'
../build/examples/example-000-lasso 1.0
../build/examples/example-001-lasso
# core dump
# ../build/examples/example-002-diabetes-lars
# ../build/examples/example-002-diabetes-lasso
../build/examples/example-003-sawtooth-lars-active-set-size 10
../build/examples/example-003-sawtooth-lars-empirical_risk 0.02
../build/examples/example-003-sawtooth-lars-lambda 1.0
../build/examples/example-003-sawtooth-lasso-empirical_risk 0.02
../build/examples/example-003-sawtooth-lasso-lambda 1.0

