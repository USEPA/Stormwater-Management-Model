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


benchmark_ver="220dev1"


test_suite_path=$2

nrtest_execute_cmd="nrtest execute"
test_app_path="apps/swmm-$3.json"
tests="tests/examples tests/extran tests/routing tests/user"
test_output_path="benchmark/swmm-$3"

nrtest_compare_cmd="nrtest compare"
ref_output_path="benchmark/swmm-${benchmark_ver}"

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
return_value=$( $nrtest_command )

if [ $1 = 'true' ]; then
  echo
  echo INFO: Comparing test and ref benchmarks
  nrtest_command="${nrtest_compare_cmd} ${test_output_path} ${ref_output_path} --rtol ${rtol_value} --atol ${atol_value}"
  echo INFO: "$nrtest_command"
  return_value=$( $nrtest_command )
fi

return $return_value
}


print_usage() {
   echo " "
   echo "run-nrtest.sh - generates artifacts for SUT and performes benchmark comparison "
   echo " "
   echo "options:"
   echo "-c,             don't compare SUT and benchmark artifacts"
   echo "-t test_path    relative path to location where test suite is staged"
   echo "-v version      version/build identifier"
   echo " "
}

# Default option values
compare='true'
test_path='nrtestsuite'
version='vXXX'

while getopts ":ct:v:" flag; do
  case "${flag}" in
    c  ) compare='false' ;;
    t  ) test_path="${OPTARG}" ;;
    v  ) version=${OPTARG} ;;
    \? ) print_usage
         exit 1 ;;
  esac
done
shift $(($OPTIND - 1))

# Invoke command
run_command="run-nrtest ${compare} ${test_path} ${version}"
echo INFO: "$run_command"
$run_command
