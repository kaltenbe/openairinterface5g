#!/bin/bash

source cmake_targets/tools/build_helper

targetfile=targets.working

if [ $# -eq 1 ]; then
  targetfile=$1
elif [ $# -gt 1 ]; then
  echo "error: only one target file supported"
  exit 1
fi

if [ ! -f $targetfile ]; then
  echo "file $targetfile: no such file or directory"
  exit 1
fi
echo "using targets in file $targetfile"
ALL_TARGETS=$(cat $targetfile)

cd cmake_targets/ran_build/build
if [ $? -ne 0 ]; then
  echo_fatal "cannot change to build_dir"
fi

rm -rf ../../../results{,.txt}
mkdir ../../../results

for t in $ALL_TARGETS; do
  #echo -n "cleanup ... "
  find . \( -name '*.a' -o -name '*.so' -o -name '*.o' \) -delete 2> /dev/null
  rm -rf CMakeFiles/{RRC_Rel15,RRC_NR_Rel16,S1AP_R15,NGAP_R15,M2AP_R14,M3AP_R14,X2AP_R15,F1AP_R16.3.1,LPP} openair2/ openair3/
  echo -ne "$t\t" | tee -a ../../../results.txt
  ( cmake ../../.. && ninja $t ) > ../../../results/$t.log 2>&1
  if [ $? -eq 0 ]; then
    echo_success "built"
    echo built >> ../../../results.txt
  else
    echo_error "failed"
    echo failed >> ../../../results.txt
  fi
done
