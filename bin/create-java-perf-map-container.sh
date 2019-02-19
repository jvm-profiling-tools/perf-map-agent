#!/bin/bash
set -e
set -x

CUR_DIR=`pwd`
PID=$1
CONTAINER_PID=`cat /proc/$PID/status | grep NSpid | cut -f3`
OPTIONS=$2
ATTACH_JAR=attach-main.jar
PERF_MAP_DIR=/home/johannes/git/self/perf-map-agent
ATTACH_JAR_PATH=$PERF_MAP_DIR/out/$ATTACH_JAR
PERF_MAP_FILE=/tmp/perf-$PID.map

if [ -z "$JAVA_HOME" ]; then
  JAVA_HOME=/usr/lib/jvm/default-java
fi

[ -d "$JAVA_HOME" ] || JAVA_HOME=/etc/alternatives/java_sdk
[ -d "$JAVA_HOME" ] || (echo "JAVA_HOME directory at '$JAVA_HOME' does not exist." && false)

sudo rm $PERF_MAP_FILE -f

# copy libperfmap.so to container
sudo cp -n $PERF_MAP_DIR/out/libperfmap.so /proc/$PID/root/tmp

# Attach and run libperfmap
sudo java -cp $ATTACH_JAR_PATH:$JAVA_HOME/lib/tools.jar net.virtualvoid.perf.AttachOnce $PID "$OPTIONS"

# Copy over perf map file from container
sudo cp /proc/$PID/root/tmp/perf-$CONTAINER_PID.map $PERF_MAP_FILE

# Make sure root owns the container
sudo chown root:root $PERF_MAP_FILE
