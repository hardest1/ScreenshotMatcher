
#include "server.hpp"

Server svr;

void stopServer()
{
  svr.stop();
}

void initializeServer(string host, int port, string serviceURL, string scriptDir, string public_folder, string results_folder)
{

  // ROUTING
  svr.Get("/test", [&](const Request &req, Response &res) {

    if(!req.has_param("id")){
      res.set_content("missing test id", "text/plain");
      return;
    }

    string test_id = req.get_param_value("id");

    string filename = test_SURF(stoi(test_id), scriptDir);

    if(filename == "no result")
    {
      res.set_content("no result", "text/html");
    }
    else
    {
      // Set result URL
      string result_url = "/results/" + filename;
      // Set response content
      res.set_content(result_url.c_str(), "text/html");
    }
    return;

    res.set_content(test_id, "text/plain");
  });

  svr.Post("/match", [&](const Request &req, Response &res) {

    _logger("POST /match");

    // Get image from posted data
    auto image_file = req.get_file_value("image_file");

    _logger("Starting match algorithm");

    // Match algorithm, returns filename of resulting image
    string filename = match( binaryToMat(image_file.content.c_str(), image_file.content.length()), results_folder );

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

  svr.listen(host.c_str(), port);
}