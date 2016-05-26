#!/bin/sh
pid=`ps -ef | grep "can_torcs_original" | grep -v "grep" | awk '{print $2}'`
echo $pid
kill -9 $pid
