#!/bin/bash

echo "=========================================================================="
echo "LEXER TESTS"
echo "=========================================================================="

make lexer
cd ./jack_samples_lexer/

echo "=========================================================================="
echo "=========================================================================="

count=0
for file in ./*.jack
do
  ../lexer $file
  diff $quiet $file"_tokens.txt" $file"_tokens_mine.txt" &>/dev/null 

  case $? in
    0)
      printf "\e[1;32m%-10s\e[0m %-10s %s\n" "PASSED:" "GetToken" $file
      ;;
    1)
      printf "\e[1;31m%-10s\e[0m %-10s %s\n" "FAILED:" "GetToken" $file
      let count++;
      ;;
    2)
      printf "\e[1;31m%-10s\e[0m %-10s %s\n" "MISSING FILE:" "GetToken" $file
      let count++;
      ;;
    *)
      echo "unknown error"
      ;;
  esac

  ../lexer $file peek
  diff $quiet $file"_tokens.txt" $file"_tokens_mine.txt" &>/dev/null 

  case $? in
    0)
      printf "\e[1;32m%-10s\e[0m \e[1;33m%-10s\e[0m %s\n" "PASSED:" "PeekToken" $file
      ;;
    1)
      printf "\e[1;31m%-10s\e[0m \e[1;33m%-10s\e[0m %s\n" "FAILED:" "PeekToken" $file
      let count++;
      ;;
    2)
      printf "\e[1;31m%-10s\e[0m \e[1;33m%-10s\e[0m %s\n" "MISSING FILE:" "PeekToken" $file
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
