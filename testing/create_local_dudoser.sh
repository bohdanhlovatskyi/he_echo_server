#!/bin/bash

#yes | sudo pacman -S sshpass openssh

docker build -f Dockerfile.worker -t sucks .
docker rm -f sucks
docker network rm hostonly
docker network create -d bridge --internal hostonly
docker run -d --network hostonly -p 22:22 --name sucks sucks

# ip is about 172.19.0.2
# to check run
#`ip a`
# and watch into bridge (etc. br-10145babadc3)

#to connect
#sshpass -p "root" ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null root@172.20.0.2 -p 22
