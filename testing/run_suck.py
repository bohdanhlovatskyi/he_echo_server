import json
import os
import subprocess
from time import sleep
from sys import argv


def get_configs(num_clients, cluster_path='cluster.json'):
    with open(cluster_path, 'r', encoding='utf-8') as f_conf:
        cluster_cnf = json.load(f_conf)['data']
    fortio_cnf = {
        'qps': 0,
        'time': '60s',
        'num_clients': num_clients,
        'server_ip': 'tcp://192.168.0.1:9000'
    }
    return cluster_cnf, fortio_cnf

class ReceiveData:
    def __init__(self, server_some):
        ethernet = "enp43s0"
        server_bin, num = server_some
        out_file = f'{server_bin}/server_{server_bin}_{num}.log'
        command = f"#!/bin/bash\n" \
                  f"rm -f {out_file}\n" \
                  f"while true; do (echo time,rMb,wMb >> {out_file}) && (nicstat -M | grep {ethernet} | tr -s ' ' | cut -d ' ' -f 1,3,4 >> {out_file}) && (echo %CPU,%MU >> {out_file}) && (top -p $(pidof {server_bin} | tr ' ' ',') | head -8 | tail -1 | tr -s '  ' | cut -d ' ' -f 10 >> {out_file}) && (pmap $(pidof {server_bin} | tr ' ' ',') | tail -1 | tr -s '  ' | cut -d ' ' -f 3 >> {out_file}) && (echo '' >> {out_file}); sleep 1; done"
        with open('track_time.sh', 'w') as f:
            f.write(command)
        self.check_server_stat = subprocess.Popen(['./track_time.sh'])

    def __del__(self):
        self.check_server_stat.terminate()


def main():
    version = argv[1]
    num_clients = int(argv[2])
    data_path = f'{version}'
    if not os.path.exists(data_path):
        # shutil.rmtree(data_path)
        os.mkdir(data_path)

    cluster_cnf, fortio_cnf = get_configs(num_clients)
    some_trash = ReceiveData((version, argv[2]))
    # num_of_sockets = int(int(num_clients) * 1.3)
    work = []
    for worker in cluster_cnf:
        json_path = f"res_{version}_{num_clients}_{worker['name']}.json"
        command_ssh = f"sshpass -p {worker['password']} ssh {worker['user']}"
        command_fortio = f"ulimit -n 50000 && fortio load -qps {fortio_cnf['qps']} -t {fortio_cnf['time']} -c {fortio_cnf['num_clients']} -json {json_path} -uniform -nocatchup {fortio_cnf['server_ip']}  2>&1 | tee tester_log.log "

        print(f'run : {command_ssh} {command_fortio}')
        some = subprocess.Popen(
            f'{command_ssh} {command_fortio}'.split(),
            stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            encoding='utf-8', universal_newlines=True, bufsize=0
        )
        work.append(some)

    print('done starting processes')
    sleep(int(fortio_cnf['time'][:-1]) + 30)

    for worker in cluster_cnf:
        json_path = f"res_{version}_{num_clients}_{worker['name']}.json"
        command_copy_json = f"sshpass -p {worker['password']} scp {worker['user']}:{json_path} {data_path}"
        print(f'run : {command_copy_json}')
        subprocess.run(command_copy_json.split())
        # command_delete_json = f"sshpass -p {worker['password']} ssh {worker['user']} rm -f {path_remote}"
        # subprocess.run(command_delete_json.split())

    print('done copying')



if __name__ == '__main__':
    main()
