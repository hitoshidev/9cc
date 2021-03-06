#!/bin/bash
try() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

try 0 "2 < 1"
try 1 "1 <= 2"
try 1 "1 <= 1"
try 0 "2 <= 1"
try 0 "1 > 2"
try 1 "2 > 1"
try 0 "1 >= 2"
try 1 "1 >= 1"
try 1 "2 >= 1"

try 1 "1 == 1"
try 0 "1 == 2"
try 0 "1 != 1"
try 1 "1 != 2"

try 47 '5+ 6*7'
try 15 '5*( 9-6)'
try 4 '(3+5) /2'
try 18 '(31+5) /2'
try 5 "-5+5*2"
echo OK