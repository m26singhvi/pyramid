#! /bin/bash

# Logic to generate the random number
#
# $RANDOM - Gives the random number
# set low and high range e.g numbers in 11 and 20
# diff = high - low
# random = low + $RANDOM % diff

rm input_p*
rm output_p*

trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM EXIT

common=26

rand1=$RANDOM
rand2=$RANDOM
rand3=$RANDOM
rand4=$RANDOM
rand5=$RANDOM

low1=101
low2=201
low3=301
low4=401
low5=501

high1=200
high2=300
high3=400
high4=500
high5=600

diff1=$(($high1-$low1))
diff2=$(($high2-$low2))
diff3=$(($high3-$low3))
diff4=$(($high4-$low4))
diff5=$(($high5-$low5))

# echo $low1 $high1 $diff1
# echo $low2 $high2 $diff2
# echo $low3 $high3 $diff3
# echo $low4 $high4 $diff4
# echo $low5 $high5 $diff5

# echo $common $(($low1+$RANDOM%$diff1)) $(($low2+$RANDOM%$diff2)) $(($low3+$RANDOM%$diff3)) $(($low4+$RANDOM%$diff4)) $(($low5+$RANDOM%$diff5)) 

ip=127.0.0.1
port=$1
groups='5 8 12 16 3'
#count=$1
count=$2
xterm -e ../../server/bin/server $port &
sleep 2
until [[ $count -lt 1 ]]; do
    xterm -e ../../client/bin/client $ip $port $common $(($low1+$RANDOM%$diff1)) $(($low2+$RANDOM%$diff2)) $(($low3+$RANDOM%$diff3)) $(($low4+$RANDOM%$diff4)) $(($low5+$RANDOM%$diff5)) &
    let count-=1
done

sleep 2

xterm -T PYRAMID -e ../../client/bin/client $ip $port CLI &

wait
