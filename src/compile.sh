#!/bin/bash
g++ -o ioperf main.cpp server.cpp client.cpp writer.cpp reader.cpp \
    -O3 -std=c++0x -W -Wall -Wshadow -Wpointer-arith -pedantic-errors
