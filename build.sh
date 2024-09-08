#!/bin/sh

set -x
rm -rf "$(pwd)"/build/*
cd "$(pwd)"/build/

cmake .. &&
make