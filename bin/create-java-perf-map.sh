#!/bin/bash
set -e
#set -x

CUR_DIR=`pwd`
PID=$1
OPTIONS=$2
ATTACH_JAR=attach-main.jar
PERF_MAP_DIR=$(dirname $(readlink -f $0))/..
ATTACH_JAR_PATH=$PERF_MAP_DIR/out/$ATTACH_JAR
PERF_MAP_FILE=/tmp/perf-$PID.map

if [ ! -d /proc/$PID ]; then
  echo "PID $PID not found"
  exit 1
fi
TARGET_UID=$(awk '/^Uid:/{print $2}' /proc/$PID/status)

if [ -z "$JAVA_HOME" ]; then
  JAVA_HOME=/usr/lib/jvm/default-java
fi

[ -d "$JAVA_HOME" ] || JAVA_HOME=/etc/alternatives/java_sdk
[ -d "$JAVA_HOME" ] || (echo "JAVA_HOME directory at '$JAVA_HOME' does not exist." && false)

sudo rm $PERF_MAP_FILE -f
(cd $PERF_MAP_DIR/out && sudo -u \#$TARGET_UID $JAVA_HOME/bin/java -cp $ATTACH_JAR_PATH:$JAVA_HOME/lib/tools.jar net.virtualvoid.perf.AttachOnce $PID "$OPTIONS")
sudo chown root:root $PERF_MAP_FILE
