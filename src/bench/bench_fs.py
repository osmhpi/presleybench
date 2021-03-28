
# schema:
#
#  Experiments ([id], starttime, endtime, user, description)
#  Series ([id], F[experiment], commitid, index, starttime, endtime, log)
#  Measurement ([series, round, cpu, node], thread, timestamp, value)

from itertools import groupby
import multiprocessing
import datetime
import signal
import time
import uuid
import sys
import pwd
import os
import re

import git
import sh

# retrieve the command to run from the command line
cmd = getattr(sh, sys.argv[1])
args = sys.argv[2:]

# the experiment description
description = sys.argv[1] + "_".join(args)

# the number of times to repeat each measurement
REPETITIONS = 10

# the duration of each run
ROUND_TIMER = 5 # seconds

class Experiment:
    #  Experiments ([id], starttime, endtime, user, description)

    def __init__(self, description):
        self._id = uuid.uuid4().hex
        self._starttime = datetime.datetime.now()
        self._endtime = None
        self._user = pwd.getpwuid(os.getuid()).pw_name
        self._description = description

        self.series = []

    def __enter__(self):
        # create experiment folder
        self._folder = description + "_" + self._id
        if not os.path.exists(self._folder):
            os.makedirs(self._folder)
        return self

    def __exit__(self, type, value, tb):
        self._endtime = datetime.datetime.now()
        experiment_file = open(os.path.join(self._folder, "experiment.csv"), "w")
        experiment_file.write(str(self) + "\n")

    def __repr__(self):
        return "%s;%s;%s;%s;%s" % (self._id, self._starttime, self._endtime, self._user, self._description)

    def Series(self, *args, **kwargs):
        series = Series(self, *args, **kwargs)
        self.series.append(series)
        return series

class Series:
    #  Series ([id], F[experiment], commitid, index, starttime, endtime, log)

    def __init__(self, experiment):
        self._experiment = experiment
        self._index = len(experiment.series)
        self._starttime = datetime.datetime.now()
        self._endtime = None
        self._log = ""

        self._measurements = []

    def __enter__(self):
        # create series folder
        self._folder = os.path.join(self._experiment._folder, str(self._index))
        if not os.path.exists(self._folder):
            os.makedirs(self._folder)
        return self

    def __exit__(self, type, value, tb):
        self._endtime = datetime.datetime.now()
        # Series Meta File
        series_file = open(os.path.join(self._folder, "series.csv"), "w")
        series_file.write(str(self) + "\n")
        
        # Measurements File
        measurements_file = open(os.path.join(self._folder, "measurements.csv"), "w")
        for measurement in self._measurements:
            measurements_file.write(str(measurement) + "\n")
        
        # Aggregated Measurements File
        aggregated_measurements_file = open(os.path.join(self._folder, "aggregates.csv"), "w")
        grouping = lambda x: x._round
        self._measurements.sort(key=grouping)
        for key, group in groupby(self._measurements, grouping):
            aggregate = 0
            for measurement in group:
                aggregate += int(measurement._value)
            aggregated_measurements_file.write(str(key) + ";" + str(aggregate) + "\n")

    def __repr__(self):
        return "%s;%s;%s;%s;%s" % (self._experiment._id, self._index, self._starttime, self._endtime, self._log)

    def Measurement(self, *args, **kwargs):
        measurement = Measurement(self, *args, **kwargs)
        self._measurements.append(measurement)
        return measurement

class Measurement:
    #  Measurement ([series, round, cpu, node], thread, timestamp, value)

    def __init__(self, series, round, cpu, node, thread, value):
        self._series = series
        self._round = round
        self._cpu = cpu
        self._node = node
        self._thread = thread
        self._timestamp = datetime.datetime.now()
        self._value = value

        # TODO: insert into database

    def __repr__(self):
        return "%s, %s, %s, %s, %s, %s, %s" % (self._series._index, self._round, self._cpu, self._node, self._thread, self._timestamp, self._value)

# prepare an event that will signal the end of the init phase of the started command
ready = multiprocessing.Event()

# begin the experiment
with Experiment(description) as experiment:

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
            except Exception:
                # FIXME: this doesn't always work. sometimes, the process stays running when the script exits on error
                process.kill()
                raise
