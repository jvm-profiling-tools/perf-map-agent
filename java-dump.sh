if [ -p $JAVA_HOME ]; then
  JAVA_HOME=/usr/lib/jvm/default-java
fi

echo $* >> output.log
sudo -u johannes java -cp attach-main.jar:$JAVA_HOME/lib/tools.jar net.virtualvoid.perf.AttachOnce $*
