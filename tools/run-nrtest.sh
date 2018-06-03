#! /bin/bash
#
#  run-nrtest.sh - Runs numerical regression test
#
#  Date Created: 11/15/2017
#
#  Author:       Michael E. Tryby
#                US EPA - ORD/NRMRL
#
#  Arguments:
#   1 - test suite path
#   2 - version/build identifier
#

run-nrtest()
{
return_value=0

test_suite_path=$1

nrtest_execute_cmd="nrtest execute"
test_app_path="apps/swmm-$2.json"
tests="tests/examples tests/extran tests/routing tests/user" 
test_output_path="benchmark/swmm-$2"

nrtest_compare_cmd="nrtest compare"
ref_output_path="benchmark/swmm-5112"

rtol_value=0.01
atol_value=0.0

# change current directory to test_suite
cd ${test_suite_path}

# clean test benchmark results
rm -rf ${test_output_path}

echo
echo INFO: Creating test benchmark
nrtest_command="${nrtest_execute_cmd} ${test_app_path} ${tests} -o ${test_output_path}"
echo INFO: "$nrtest_command"
$nrtest_command

echo
echo INFO: Comparing test and ref benchmarks
nrtest_command="${nrtest_compare_cmd} ${test_output_path} ${ref_output_path} --rtol ${rtol_value} --atol ${atol_value}"
echo INFO: "$nrtest_command"
$nrtest_command

}

run-nrtest $1 $2
