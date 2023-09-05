#!/bin/env sh
out="bin"
in="main.cpp"

g++ $in -lX11 -lcairo --std=c++11 -O1 -o $out -fpermissive
[[ $? == 0 ]] && ./$out

