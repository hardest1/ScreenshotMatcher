#include <iostream>
#include <fstream>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <limits.h>

#include "httplib/httplib.h"
#include "daemonize/daemonize.hpp"
#include "matchscreenshot/matchscreenshot.hpp"

using namespace std;
using namespace httplib;

Server svr;

// Server Config
string host;
int port;
bool startAsDaemon;