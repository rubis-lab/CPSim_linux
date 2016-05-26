#!/bin/sh

export SIMULATOR_PATH=/home/rubis/Downloads/simulator
rm -rf *.hh task_created.cpp simulator/
mkdir simulator
./generating_codes
mv *.hh simulator/
mv task_created.cpp simulator/
cp $SIMULATOR_PATH/engine/* simulator/
cd simulator
make
./real-time_simulator
