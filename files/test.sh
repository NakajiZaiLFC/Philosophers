#!/bin/zsh
make
make clean
valgrind --tool=helgrind -q ./philo 3 100 10 10 3
