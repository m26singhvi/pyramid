trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM EXIT

ip=127.0.0.1
port=51729
groups='5 8 12 16 3'
count=$1
xterm -e ./server/bin/server $port &
until [[ $count -lt 0 ]]; do
    xterm -e ./client/bin/client $ip $port $groups &
    let count-=1
done

xterm -e ./client/bin/client $ip $port CLI &

wait
