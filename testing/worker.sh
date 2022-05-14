#!/bin/bash

#ssh_ip="root@172.20.0.2"
#ssh_port="22"
#ssh_psw="root"
#server_ip="tcp://172.20.0.1:9000"
#output_file="sasat1.json"
#
#fortio="/go/bin/fortio"
#time="5s"
#qps="100000"
#threads="10"
#
#fortio_cmd="${fortio} load -qps ${qps} -t ${time} -c ${threads} -json ./${output_file} ${server_ip}"


#if [ "$#" -ne 9 ] || ! [ -d "$1" ]; then
if [ "$#" -ne 9 ]; then
  echo "Usage: $0 <ssh_ip> <ssh_port> <ssh_psw> <server_ip> <output_file>" >&2
  exit 1
fi

ssh_ip=$1
ssh_port=$2
ssh_psw=$3
server_ip=$4
output_file=$5

fortio=$6
time=$7
qps=$8
threads=$9

tmp_file="tmp.json"

fortio_cmd="${fortio} load -qps ${qps} -t ${time} -c ${threads} -json ./${tmp_file} ${server_ip}"

sshpass -p "${ssh_psw}" ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null "${ssh_ip}" -p "${ssh_port}" "${fortio_cmd}"
sshpass -p "${ssh_psw}" scp -P "${ssh_port}" "${ssh_ip}:/root/${tmp_file}" .
mv "${tmp_file}" "${output_file}"

#sshpass -p "root" ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null root@172.20.0.2 -p 22
#ssh root@172.20.0.2 -p 22



#sshpass -p 'password' ssh bubuntyk@localhost -p 22
#
#docker run -d -p 2222:22 -p 8080:8080 sucks
##sshpass -p 'root' ssh root@localhost -p 2222
#
#sshpass -p 'root' ssh root@localhost -p 2222 /go/bin/fortio load -qps 1000 -t 2s -c 10 -json "sasat.json" tcp://127.0.0.1:9000
#sshpass -p 'root' scp -P 2222 ssh root@localhost:~/sasat.json ./data/sasat.json
#sshpass -p 'root' scp -P 2222 root@localhost:~/sasat.json ./data/
#
#sshpass -p "root" ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null root@localhost -p 2222
#sshpass -p "root" ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null root@localhost -p 2222 /go/bin/fortio load -qps 1000 -t 2s -c 10 -json "sasat.json" tcp://127.0.0.1:9000
#
#docker network create -d bridge --internal hostonly
#docker run -d --network hostonly -p 2222:22 --name sucks sucks
#docker run -d -p 2222:22 --name sucks sucks
#
##sshpass -p 'test' ssh -i "${ssh_key}" test@localhost -p 22 \
##    fortio load -qps 1000 -t 2s -c 10 -json "sasat.json" tcp://127.0.0.1:9000
##sshpass -p 'test' scp -P 22 test@localhost:~/fifi.txt ./data/fifi.txt
#
##sshpass -p 'test' ssh -i "${ssh_key}" testls@localhost -p 22 \
##  docker run fortio load -qps 1000 -t 2s -c 10 -json "sasat.json" tcp://127.0.0.1:9000
##sshpass -p 'test' scp -P 22 test@localhost:~/sasat.json ./data/sasat.json
