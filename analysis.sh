#!/usr/bin/env bash

# dieharder -a -g 205 -f ciphertext.aes.xor | tee prng.205.ciphertext.aes.xor.txt

test_file="ciphertext.aes.xor"
test=("006" "014" "052" "205" "206" "501")

if [ -f ${test_file} ]
then
  for k in ${test[@]}
  do
    echo "dieharder -a -g ${k} -f ${test_file} | tee analysis.${k}.${test_file}.txt"
  done
else
  exit 1
fi

exit 0

# end of script
