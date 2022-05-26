#!/bin/bash

#ssh_ip="root@172.20.0.2"
#ssh_port="22"
#ssh_psw="root"
#server_ip="tcp://172.20.0.1:9000"
#output_file="res1.json"
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

data_dir="./data"
fortio_cmd="${fortio} load -qps ${qps} -t ${time} -c ${threads} -json ${output_file} ${server_ip}"


sshpass -p "${ssh_psw}" ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null "${ssh_ip}" -p "${ssh_port}" "${fortio_cmd}"
sshpass -p "${ssh_psw}" scp -P "${ssh_port}" "${ssh_ip}:/root/${output_file}" .
mv "${output_file}" "${data_dir}/${output_file}"


echo "sshpass -p ${ssh_psw} ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null ${ssh_ip} -p ${ssh_port} ${fortio_cmd}
sshpass -p ${ssh_psw} scp -P ${ssh_port} ${ssh_ip}:/root/${output_file} .
mv ${output_file} ${data_dir}/${output_file}" > "data2${output_file}.txt"

#ssh root@172.22.0.2 -p 22
#sshpass -p root ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null root@172.22.0.2 -p 22 /go/bin/fortio load -qps 10000 -t 2s -c 6 -json pipi1.json tcp://172.22.0.1:9000
sshpass -p root scp -P 22 ./bin root@172.22.0.2:/root/pipi1.json .
#mv pipi1.json ./data/pipi1.json

