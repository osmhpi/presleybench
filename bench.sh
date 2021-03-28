#! /bin/bash

python3 src/bench/bench_fs.py ./simplebench
python3 src/bench/bench_fs.py ./simplebench -i -Ibplus
python3 src/bench/bench_fs.py ./simplebench -e -i -Ibplus
