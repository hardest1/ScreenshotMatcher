#include <iostream>
#include <fstream>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <limits.h>
#include <thread>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

#include <libappindicator/app-indicator.h>

#include "qrcodegen/QrCode.hpp"
#include "httplib/httplib.h"
#include "daemonize/daemonize.hpp"
#include "matchscreenshot/matchscreenshot.hpp"
#include "server/server.hpp"
#include "logger/logger.hpp"

using namespace std;
using namespace httplib;

using std::uint8_t;
using qrcodegen::QrCode;
using qrcodegen::QrSegment;

string scriptDir;
string results_folder;
string serviceURL;

// Server Config
string host;
int port;
bool startAsDaemon;

// Tray
GtkWidget*      indicator_menu;
GtkActionGroup* action_group;
GtkUIManager*   uim;
AppIndicator* indicator;
GError* gerror = NULL;