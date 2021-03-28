#!/usr/bin/env python3

import sys
import pwd 
import os
import signal
import subprocess
import threading
import time
from datetime import datetime
import uuid

import git
import psycopg2


CONNECTION = "postgres://postgres:password@192.168.42.38:5432/presley"
SQL_INIT = "INSERT INTO presley.experiments (id, starttime, endtime, commitid, name, user) VALUES (%s, %s, %s, %s, %s, %s);"
SQL_BENCH = "INSERT INTO presley.queries (timestamp, thread_id, node_id, cpu_id, round, count, experiment) VALUES (%s, %s, %s, %s, %s, %s, %s);"


def timer(popen):
    time.sleep(20)
    while True:
        print("Sending Signals")
        popen.send_signal(signal.SIGUSR2)
        time.sleep(5)
        popen.send_signal(signal.SIGUSR1)
        time.sleep(1)


def execute():
    popen = subprocess.Popen(sys.argv[2:], stdout=subprocess.PIPE, universal_newlines=True)
    thread = threading.Thread(target = timer, args = [popen])
    thread.start()
    for stdout_line in iter(popen.stdout.readline, ""):
        yield stdout_line
    popen.stdout.close()
    thread.join()
    return_code = popen.wait()
    if return_code:
        raise subprocess.CalledProcessError(return_code, sys.argv[2:])


def main():
    repo = git.Repo(search_parent_directories=True)
    sha = repo.head.object.hexsha
    experiment_id = uuid.uuid4().hex
    os_username = pwd.getpwuid(os.getuid()).pw_name
    with psycopg2.connect(CONNECTION) as conn:
        cur = conn.cursor()
        try:
            data = (experiment_id, datetime.now(), datetime.now(), sha, sys.argv[1], os_username)
            cur.execute(SQL_INIT, data)
        except (Exception, psycopg2.Error) as error:
            print("Postgres Error: " + error)
        conn.commit()
        for stdout in execute():
            if stdout == "threadid,nodeid,cpuid,round,counter":
                print(stdout)
                continue
            if stdout == "resetting thread counters":
                print(stdout)
                continue

            print(stdout)
            if len(stdout.split(",")) == 5:
                threadid, nodeid, cpuid, bench_round, counter = stdout.split(",")
                try:
                    #timestamp, thread_id, node_id, cpu_id, round, count, experiment
                    data = (datetime.now(), threadid, nodeid, cpuid, bench_round, counter, experiment_id)
                    cur.execute(SQL_BENCH, data)
                except (Exception, psycopg2.Error) as error:
                    print("Postgres Error: " + error.pgerror)
                conn.commit()


if __name__ == '__main__':
    main()
