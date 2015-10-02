trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM EXIT

make
ip=127.0.0.1
port=$1
groups='5 8 12 16 3'
count=$2
cd server
xterm -e ./bin/server $port &
sleep 5
cd ../client
until [[ $count -lt 0 ]]; do
    xterm -e ./bin/client $ip $port $groups &
    let count-=1
done
sleep 5
xterm -e ./bin/client $ip $port CLI &

wait
