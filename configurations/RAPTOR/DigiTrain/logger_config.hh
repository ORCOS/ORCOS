#ifndef LOGGERCONFIG_HH_
#define LOGGERCONFIG_HH_



 //Global logger config

enum Level {FATAL=0,ERROR=1,WARN=2,INFO=3,DEBUG=4,TRACE=5};

enum Prefix {
KERNEL=INFO,
MEM=ERROR,
PROCESS=WARN,
SCHEDULER=WARN,
SYNCHRO=ERROR,
SYSCALLS=WARN,
DB=ERROR,
HAL=ERROR,
ARCH=ERROR,
FILESYSTEM=ERROR,
COMM=ERROR
};
#endif

