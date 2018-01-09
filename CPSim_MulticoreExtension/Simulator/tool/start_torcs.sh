#!/bin/sh

export TORCS_CAN_PATH=/home/rubis/can_demo
cd $TORCS_CAN_PATH
./can_torcs_original&
torcs&
