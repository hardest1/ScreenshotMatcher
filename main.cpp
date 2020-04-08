// source: https://docs.opencv.org/3.4/d7/dff/tutorial_feature_homography.html
// or: https://github.com/opencv/opencv/blob/3.4/samples/cpp/tutorial_code/features2D/feature_homography/SURF_FLANN_matching_homography_Demo.cpp



// sources:
// getIPAddress: https://gist.github.com/quietcricket/2521037
// getExePath: http://www.cplusplus.com/forum/general/11104/
// httplib: https://github.com/yhirose/cpp-httplib
// daemonize: https://github.com/pasce/daemon-skeleton-linux-c


#include "main.hpp"

void _logger( string message, bool forceStdout = false )
{
  if(!startAsDaemon || forceStdout)
  {
    cout << message << endl;
  }
  if(startAsDaemon)
  {
    syslog(LOG_NOTICE, "%s", message.c_str());
  }
}

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

int main( int argc, char* argv[] )
{

  // Get Script Dir
  string scriptDir = getexepath();
  string public_folder = scriptDir + "/www";
  string results_folder = scriptDir + "/www/results";

  // Server Config
  host = "0.0.0.0";
  port = 49049;
  startAsDaemon = false;

  string serviceIP = getIPAddress();
  string serviceURL = "http://" + serviceIP + ":" + to_string(port);
  
  // ROUTING

  svr.Post("/match", [&](const Request &req, Response &res) {

    _logger("POST /match");

    // Get image from posted data
    auto image_file = req.get_file_value("image_file");

    _logger("Starting match algorithm");

    // Match algorithm, returns filename of resulting image
    string filename = match( binaryToMat(image_file.content.c_str(), image_file.content.length()) );

    _logger("Finished matching");

    if(filename == "no result")
    {
      // Set response content
      res.set_content("no result", "text/html");
    }
    else
    {

      // Set result URL
      string result_url = "/results/" + filename;

      // Set response content
      res.set_content(result_url.c_str(), "text/html");

    }

  });

  // Result list
  svr.Get("/result-list", [&](const Request& req, Response& res) {
    DIR *dir;
    struct dirent *ent;
    string result_list_str = "";
    // Return a list of all files in results folder
    if ((dir = opendir (results_folder.c_str())) != NULL) {
      while ((ent = readdir (dir)) != NULL) {
        result_list_str += ent->d_name;
        result_list_str += "\n";
      }
      closedir (dir);
    }
    res.set_content(result_list_str.c_str(), "text/plain");
  });

  // Get URL
  svr.Get("/get-url", [&](const Request& req, Response& res) {
    res.set_content( serviceURL, "text/plain");
  });

  // Heartbeat
  svr.Get("/heartbeat", [&](const Request& req, Response& res) {
    res.set_content("ok", "text/plain");
  });

  // Stop server / daemon
  svr.Get("/stop", [&](const Request& req, Response& res) {
    svr.stop();
    res.set_content("ok", "text/html");
  });

  // Mount www folder with html files and results
  svr.set_mount_point("/", public_folder.c_str());

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

  // Start HTTP server
  svr.listen(host.c_str(), port);

  // Finish up
  _logger("ScreenshotServer closing.. bye bye");

  // Close syslog if daemonized
  if(startAsDaemon){ closelog(); }

  // Exit with success
  return(EXIT_SUCCESS);
}