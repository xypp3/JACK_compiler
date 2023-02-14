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
  diff $quiet $file"_tokens.txt" $file"_tokens_mine.txt" &>/dev/null 

  case $? in
    0)
      echo -e "\e[1;32m PASSED:       $file \e[0m";
      ;;
    1)
      echo -e "\e[1;31m FAILS:        $file \e[0m";
      let count++;
      ;;
    2)
      echo -e "\e[1;31m MISSING FILE: $file"_tokens_mine.txt"\e[0m";
      let count++;
      ;;
    *)
      echo "unknown error"
      ;;
  esac

  # rerun diff with verbose on fail to generate text differences
        #${1:false} takes either $1 or undefined and undefined != --verbose
  if [ ${1:-undefined} == "--verbose" ]; then
    diff $quiet $file"_tokens.txt" $file"_tokens_mine.txt";
  fi

done

echo "=========================================================================="
echo -e "\e[1;31m $count\e[0m TESTS FAILED"
echo "=========================================================================="
