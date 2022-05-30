#!/bin/bash

for i in 9 73 625 2500 10000
#for i in 5000 10000
#for i in 5 10 20
do
  echo "Run : python run_suck.py $1 $i"
  echo
  python run_suck.py $1 "$i"
  echo
done
