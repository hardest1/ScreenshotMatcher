
#include "server.hpp"

Server svr;

void stopServer()
{
  svr.stop();
}

void initializeServer(string host, int port, string serviceURL, string scriptDir, string public_folder, string results_folder)
{

  // ROUTING

  svr.Post("/feedback", [&](const Request &req, Response &res) {

  });

  svr.Get("/test", [&](const Request &req, Response &res) {


    if(!req.has_param("image")){
      res.set_content("missing image parameter", "text/plain");
      return;
    }

    if(!req.has_param("algo")){
      res.set_content("missing algorithm", "text/plain");
      return;
    }

    if(!req.has_param("n")){
      res.set_content("missing n parameter", "text/plain");
      return;
    }

    string algo_id = req.get_param_value("algo");
    string n = req.get_param_value("n");
    string image = req.get_param_value("image");

    string filename = test_algos(stoi(image), stoi(algo_id), stoi(n), scriptDir);

    if(filename == "no result")
    {
      res.set_content("no result", "text/html");
    }
    else
    {
      // Set result URL
      string result_url = "/results/tests/" + filename;
      // Set response content
      res.set_content(result_url.c_str(), "text/html");
    }
    return;
  });

  svr.Post("/match", [&](const Request &req, Response &res) {

    _logger("POST /match");

    // Get image from posted data
    auto image_file = req.get_file_value("image_file");
    

    // Match algorithm, returns filename of resulting image
    string uid;
    bool hasResult;
    string filename;
    tie(uid, hasResult, filename) = match( image_file.content.c_str(), image_file.content.length(), results_folder );

    _logger(uid);

    if(!hasResult)
    {
      // Set response content
      res.set_content(uid, "text/html");
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