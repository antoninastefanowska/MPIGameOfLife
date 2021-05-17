#!/bin/bash
#SBATCH -n 8
#SBATCH -o game_of_life_frames.out
#SBATCH -e game_of_life_frames.err

mpiexec ./game_of_life_parallel1.o 40 100 0 -e
