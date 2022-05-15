# dUrKa

### Usage
- to create docker demon run `create_local_dudoser.sh`
- after check bridge ip by running `ip a` and watch at 
etc. 'br-a5708fa7c1cd' ip address. Let it be `172.21.0.1`.
- In file `cluster.csv` change server ip to `tcp://172.21.0.1:9000`
and ssh_ip to `root@172.21.0.2,22`
- Run `python3.10 ./master.py` for start testing.
- results if folder` data.
- Run `cleanup.sh` to close active docker containers.
- dieeeeeeeeeee