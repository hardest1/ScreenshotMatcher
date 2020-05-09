#include <iostream>
#include <ctime>
#include <sstream>
#include <random>
#include <string>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "opencv2/core.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"

using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;

struct ScreenShot;

string takeScreenshot(string uid);
tuple<string, bool, string> match(const char* data, int length, string result_dir);
string test_algos(int image, int algo, int n, string scriptDir);
unsigned int random_char();
std::string generate_hex(const unsigned int len);
