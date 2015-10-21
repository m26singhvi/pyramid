#! /bin/bash

#echo $0 $1 $2 $3

var=$(wc -l $3)
arr=($var)

count=$(($arr / $1))
modulus=$(($arr % $1))
zero=0
one=1

#echo $modulus

if [ $modulus != $zero ]
then
	echo "mod not zero"
	count=$(($count + $one))
fi

#echo $count
#echo /tmp/cohort/job_$2/prob/

mkdir -p /tmp/cohort/job_$2/prob/

split -dl $count $3 /tmp/cohort/job_$2/prob/p
