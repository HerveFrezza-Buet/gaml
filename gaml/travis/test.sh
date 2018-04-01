#!/bin/bash

# To list the examples :
# cd gaml/travis
# ls -l ../build/examples/example-* | awk '{print $NF}'

../build/examples/example-000-000-overview
../build/examples/example-001-001-handcrafted-parser
../build/examples/example-001-001-JSON-parser
../build/examples/example-001-002-indexed-dataset
../build/examples/example-001-003-set-manipulations
../build/examples/example-001-004-span
../build/examples/example-001-005-tree
../build/examples/example-001-006-score
../build/examples/example-001-007-score
../build/examples/example-002-001-confusion
../build/examples/example-002-002-roc
../build/examples/example-002-003-cross-validation
../build/examples/example-002-004-bootstrapping
../build/examples/example-002-005-bagging
../build/examples/example-002-006-scorer
../build/examples/example-003-001-multidimension
../build/examples/example-003-002-multiclass
../build/examples/example-004-001-variable-projection
# These two examples request a choice in std::cin
#../build/examples/example-004-002-filter-based-selection
#../build/examples/example-004-003-wrapper-based-selection
../build/examples/example-005-students generate
../build/examples/example-005-students run
