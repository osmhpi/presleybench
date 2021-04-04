
# schema:
#
#  Experiments ([id], starttime, endtime, user, description)
#  Series ([id], F[experiment], commitid, index, starttime, endtime, log)
#  Measurement ([series, round, cpu, node], thread, timestamp, value)

import multiprocessing
import datetime
import signal
import time
import uuid
import sys
import pwd
import os
import re

import psycopg2
import git
import sh

class Experiment:
    #  Experiments ([id], starttime, endtime, user, description)

    def __init__(self, description, db):
        self._id = uuid.uuid4().hex
        self._starttime = datetime.datetime.now()
        self._endtime = None
        self._user = pwd.getpwuid(os.getuid()).pw_name
        self._description = description
        self._db = db

        self.series = []

    def __enter__(self):
        # TODO: insert to database
        return self

    def __exit__(self, type, value, tb):
        self._endtime = datetime.datetime.now()
        # TODO: update end timestamp in database

    def __repr__(self):
        return "Experiment(%s, %s, %s, %s, %s)" % (self._id, self._starttime, self._endtime, self._user, self._description)

    def Series(self, *args, **kwargs):
        series = Series(self, *args, **kwargs, db=self._db)
        self.series.append(series)
        return series

class Series:
    #  Series ([id], F[experiment], commitid, index, starttime, endtime, log)

    def __init__(self, experiment, db):
        self._id = uuid.uuid4().hex
        self._experiment = experiment
        # ASSUMPTION: the current working directory is located within a git repository
        self._commitid = git.Repo(search_parent_directories=True).head.object.hexsha
        self._index = len(experiment.series)
        self._starttime = datetime.datetime.now()
        self._endtime = None
        self._log = ""
        self._db = db

        self.measurements = []

    def __enter__(self):
        # TODO: insert to database
        return self

    def __exit__(self, type, value, tb):
        self._endtime = datetime.datetime.now()
        # TODO: update end timestamp in database

    def __repr__(self):
        return "Series(%s, %s, %s, %s, %s, %s, %s)" % (self._id, self._experiment._id, self._commitid, self._index, self._starttime, self._endtime, self._log)

    def Measurement(self, *args, **kwargs):
        measurement = Measurement(self, *args, **kwargs, db=self._db)
        self.measurements.append(measurement)
        return measurement

class Measurement:
    #  Measurement ([series, round, cpu, node], thread, timestamp, value)

    def __init__(self, series, round, cpu, node, thread, value, db):
        self._series = series
        self._round = round
        self._cpu = cpu
        self._node = node
        self._thread = thread
        self._timestamp = datetime.datetime.now()
        self._value = value
        self._db = db

        # TODO: insert into database

    def __repr__(self):
        return "Measurement(%s, %s, %s, %s, %s, %s, %s)" % (self._series._id, self._round, self._cpu, self._node, self._thread, self._timestamp, self._value)

# retrieve the command to run from the command line
cmd = getattr(sh, sys.argv[1])
args = sys.argv[2:]

# prepare an event that will signal the end of the init phase of the started command
ready = multiprocessing.Event()

# the experiment description
description = """
    generic
"""

# the number of times to repeat each measurement
REPETITIONS = 10

# the duration of each run
ROUND_TIMER = 5 # seconds

# establish database connection
CONNECTION = "postgres://postgres:password@192.168.42.38:5432/presley"
with psycopg2.connect(CONNECTION) as conn:

    cur = conn.cursor()

    # begin the experiment
    with Experiment(description, cur) as experiment:

        print(experiment)

        # for 10..10000000, increasing exponentially
        for size in (10**i for i in range(2, 8)):

            print(size)

            # begin a new series
            with experiment.Series() as series:

                print(series)

                ready.clear()

                def process_output(line):
                    series._log += line

                    sys.stdout.write("OUT: [%s] %s" % (datetime.datetime.now(), line))
                    if "finished setup" in line:
                        ready.set()

                    match = re.match("(?P<thread>[0-9]*),(?P<node>[0-9]*),(?P<cpu>[0-9]*),(?P<round>[0-9]*),(?P<value>[0-9]*)", line)
                    if match is not None:
                        measurement = series.Measurement(**(match.groupdict()))
                        print(measurement)

                # run the measuree
                try:
                    process = cmd(args + ["-r", str(size)], _bg=True, _new_session=False, _out=process_output, _err=process_output)
                    ready.wait()

                    for _ in range(REPETITIONS):
                        time.sleep(1)
                        process.signal(signal.SIGUSR2)
                        time.sleep(ROUND_TIMER)
                        process.signal(signal.SIGUSR1)

                    process.signal(signal.SIGINT)
                    process.wait()
                except KeyboardInterrupt:
                    process.signal(signal.SIGINT)
                    process.wait()
                    raise
                except Exception:
                    # FIXME: this doesn't always work. sometimes, the process stays running when the script exits on error
                    process.kill()
                    raise
