// source: https://docs.opencv.org/3.4/d7/dff/tutorial_feature_homography.html
// or: https://github.com/opencv/opencv/blob/3.4/samples/cpp/tutorial_code/features2D/feature_homography/SURF_FLANN_matching_homography_Demo.cpp

// sources:
// getIPAddress: https://gist.github.com/quietcricket/2521037
// getExePath: http://www.cplusplus.com/forum/general/11104/
// httplib: https://github.com/yhirose/cpp-httplib
// daemonize: https://github.com/pasce/daemon-skeleton-linux-c

#include "main.hpp"

// ## SECTION Utility functions

// Open file (like double click in explorer)
void openFile(string filename)
{
  string sh = "xdg-open " + filename;
  system(sh.c_str());
}

// Create QR code, save as svg and show image file
void createAndShowQrCode(const char *text)
{
  const QrCode::Ecc errCorLvl = QrCode::Ecc::HIGH;  // Error correction level
	const QrCode qr = QrCode::encodeText(text, errCorLvl);
	const string svg = qr.toSvgString(3);
  time_t t = time(0);
  string filename = "/tmp/qrcode-" + to_string(t) + ".svg";
  std::ofstream file(filename);
  file << svg;
  file.close();
  openFile(filename);
}

// Get current ip address
string getIPAddress(){
    string ipAddress = "127.0.0.1";
    struct ifaddrs *interfaces = NULL;
    struct ifaddrs *temp_addr = NULL;
    int success = 0;
    // retrieve the current interfaces - returns 0 on success
    success = getifaddrs(&interfaces);
    if (success == 0) {
        // Loop through linked list of interfaces
        temp_addr = interfaces;
        while(temp_addr != NULL) {
            if(temp_addr->ifa_addr->sa_family == AF_INET) {
                if(strcmp(temp_addr->ifa_name, "en0")){
                    ipAddress = inet_ntoa(((struct sockaddr_in*)temp_addr->ifa_addr)->sin_addr);
                }
            }
            temp_addr = temp_addr->ifa_next;
        }
    }
    // Free memory
    freeifaddrs(interfaces);
    return ipAddress;
}

// get script path
string getexepath()
{
  char result[ PATH_MAX ];
  ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
  string filePath = string( result, (count > 0) ? count : 0 );

  string dirPath = "";
  vector<string> splits;
  istringstream iss(filePath);
  string s;

  while ( getline( iss, s, '/' ) )
  {
    if( s == "" ) continue;
    splits.push_back(s);
  }

  splits.pop_back();

  for ( string const& value : splits) { dirPath = dirPath + "/" + value; }

  return dirPath;
}

// ## END SECTION Utility functions

// ---------------------------------------

// ## SECTION Tray configuration

// Set tray icon
#define TRAY_ICON "folder-pictures-symbolic"

// Define tray menu actions

static void action_pair(GtkAction *action)
{
  _logger("Pairing");
  createAndShowQrCode(serviceURL.c_str());
}

static void action_results(GtkAction *action)
{
  _logger("Results");
  openFile(results_folder);
}

static void quit_application(GtkAction *action)
{
  _logger("Quit");
  stopServer();
  gtk_main_quit();
}

// Define tray menu

static GtkActionEntry entries[] = {
    {"Pair",  "device-pair",        "_Pair device",  "<control>P",
     "Show QR code to pair device",  G_CALLBACK(action_pair)},
    {"Results", "results-show",     "_Show Results", "<control>R",
     "Show all results",             G_CALLBACK(action_results)},
    {"Quit", "application-exit",    "_Quit", "<control>Q",
     "Exit the application",         G_CALLBACK(quit_application)},
};

static guint n_entries = G_N_ELEMENTS(entries);

static const gchar *ui_info =
"<ui>"
"  <popup name='IndicatorPopup'>"
"    <menuitem action='Pair' />"
"    <menuitem action='Results' />"
"    <menuitem action='Quit' />"
"  </popup>"
"</ui>";

// ## END SECTION Tray configuration

// ---------------------------------------

// ## MAIN SCRIPT

int main( int argc, char* argv[] )
{
  // Server Config
  host = "0.0.0.0";
  port = 49049;
  startAsDaemon = true;

  // Get Script Dir
  string scriptDir = getexepath();
  string public_folder = scriptDir + "/www";
  results_folder = scriptDir + "/www/results";

  string serviceIP = getIPAddress();
  serviceURL = "http://" + serviceIP + ":" + to_string(port);
  
  // Daemonize process
  if(startAsDaemon)
  {
    if( daemonize("screenshot-server", "/tmp", NULL, NULL, NULL) != 0 )
    {
      _logger("ScreenshotServer error: daemonize failed");
      exit(EXIT_FAILURE);
    }
  }
  
  _logger("ScreenshotServer starting up");

  // Initialize GTK
  gtk_init(&argc, &argv);


  // Set up tray menu

  action_group = gtk_action_group_new("AppActions");
  gtk_action_group_add_actions(action_group, entries, n_entries,
                                NULL);

  uim = gtk_ui_manager_new();
  gtk_ui_manager_insert_action_group(uim, action_group, 0);

  if (!gtk_ui_manager_add_ui_from_string(uim, ui_info, -1, &gerror)) {
      g_message("Failed to build menus: %s\n", gerror->message);
      g_error_free(gerror);
      gerror = NULL;
  }

  // Set up tray symbol
  indicator = app_indicator_new("screenshotmatcher-tray",
                                TRAY_ICON,
                                APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

  indicator_menu = gtk_ui_manager_get_widget(uim, "/ui/IndicatorPopup");

  app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
  app_indicator_set_attention_icon(indicator, "indicator-messages-new");

  app_indicator_set_menu(indicator, GTK_MENU(indicator_menu));
  
  // Init HTTP server
  std::thread t_server([public_folder]{initializeServer(host, port, serviceURL, public_folder, results_folder);});

  // Start tray event loop
  gtk_main();

  // Start HTTP server thread
  t_server.join();

  // Finish up
  _logger("ScreenshotServer closing.. bye bye");

  // Close syslog if daemonized
  if(startAsDaemon){ closelog(); }

  // Exit with success
  return(EXIT_SUCCESS);
}