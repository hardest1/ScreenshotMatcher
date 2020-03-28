#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <syslog.h>

int daemonize(const char* name, const char* path, const char* outfile, const char* errfile, const char* infile );