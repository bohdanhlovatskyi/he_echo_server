import subprocess
from subprocess import Popen

import os
from os import path

# from typing import List

import json

import pandas as pd


class Server:
    def __init__(self, server_version=1, server_port=9000):
        self.server_version = server_version
        self.server_port = server_port
        self.__server = None

    def start(self):
        os.chdir("..")
        server_bin_path = "./bin/start_server"
        if not path.exists(server_bin_path):
            subprocess.run(["./compile.sh", "-d", "-O", "-R"])
        data = tuple(map(str, [server_bin_path, self.server_port, self.server_version]))
        self.__server = Popen(data, stdout=subprocess.PIPE)
        os.chdir("./testing")
        print("Successfully start server!")

    def terminate(self):
        self.__server.terminate()
        print("Successfully terminate server!")


class Workers:
    def __init__(self):
        self.workers_info = None
        self.workers_fut = []
        # self.data_folder = "./data/"
        self.worker_sh = "./worker.sh"

    def add_workers(self, workers_data_path="cluster.csv"):
        self.workers_info = pd.read_csv(workers_data_path)
        # self.workers_info["output_file"] = self.data_folder + self.workers_info["output_file"]

    def start(self):
        for i, worker in self.workers_info.iterrows():
            data = tuple(map(str, [self.worker_sh, *tuple(worker)]))
            print(' '.join(data))
            self.workers_fut.append(
                Popen(data, stdout=subprocess.PIPE)
            )
        print("All workers begin working")

    def finish(self):
        print("wanting for ALL workers")
        for worker in self.workers_fut:
            _ = worker.wait()
        print("Workers finish suppose successfully!")

    def marge_jsons(self):
        res = {}
        for res_path in self.workers_info["output_file"]:
            with open(res_path, "r") as f:
                tmp = json.load(f)["DurationHistogram"]["Data"]


def main():
    server = Server()
    server.start()

    workers = Workers()
    workers.add_workers()

    workers.start()
    workers.finish()

    server.terminate()


if __name__ == '__main__':
    main()
