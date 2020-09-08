#!/bin/bash

# sanity checks
if [ ! -f bench.py ]; then
  echo "ERROR: this script expects to be executed in src/bench" >&2
  exit 1
fi

if [ -n "$VIRTUAL_ENV" ]; then
  echo "ERROR: there is already an activated virtualenv" >&2
  exit 1
fi

if [ -e .venv ]; then
  echo "ERROR: directory '.venv' already exists" >&2
  exit 1
fi

# create virtualenv, require python3
virtualenv -p python3 .venv || exit

# source
source .venv/bin/activate || exit

# install dependencies
pip install -r requirements.txt || exit

# done.
echo ""
echo "======================"
echo " all done."
echo " please activate the virtualenv by running:"
echo "   source .venv/bin/activate"
