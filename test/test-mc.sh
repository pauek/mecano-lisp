#!/bin/bash

real=$(mktemp real.XXXXXX)
expected=$(mktemp expected.XXXXXX)
errors=$(mktemp errors.XXXXXX)

function test-diff() {
  awk '{ 
    other = "[nothing]";
    getline other < "'$real'";
    if (other == $0) printf(".");
    else {
      printf("x");
      print "'$1': "$0" != "other >> "'$errors'";
    }
  }' $expected
  echo
}

function nlines() {
  wc -l $1 | cut -d' ' -f1
}

for f in $*; do
  file=$(basename $f)
  echo -en ${file%.mc}"\t"
  opts=$(sed -n '1p' $f | grep '^##' | cut -c3-)
  grep -v '^##' $f | grep '#' | sed 's/^.*# //' > $expected
  ./mecano $opts $f > $real
  test-diff ${file%.mc}
done

rm $real
rm $expected
if [ $(nlines $errors) != "0" ]; then echo; fi
cat $errors
rm $errors