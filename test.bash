#!/bin/bash

echo "=========================================================================="
echo "LEXER TESTS"
echo "=========================================================================="


make lexer
cd ./lexer_jack_samples/

echo "=========================================================================="
echo "=========================================================================="

count=0
for file in ./*.jack
do
  ../lexer $file
  # DELETE DEV NULL TO SHOW DIFFERENCES
  if ! diff $quiet $file"_tokens.txt" $file"_tokens_mine.txt" &>/dev/null; then
    echo -e "\e[1;31m $file FAILED\e[0m"
    let count++

  else
    echo -e "\e[1;32m $file PASSED\e[0m"
  fi

done

echo "=========================================================================="
echo -e "\e[1;31m $count\e[0m TESTS FAILES"
echo "=========================================================================="
