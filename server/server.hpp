#include <iostream>
#include <fstream>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <limits.h>

#include "../logger/logger.hpp"
#include "../matchscreenshot/matchscreenshot.hpp"
#include "../httplib/httplib.h"

using namespace httplib;
using namespace std;

void stopServer();
void initializeServer(string host, int port, string serviceURL, string public_folder, string results_folder);