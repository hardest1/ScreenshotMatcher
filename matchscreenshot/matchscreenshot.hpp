#include <iostream>
#include <ctime>

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

Mat matchScreenshot(Mat photo, Mat screenshot, Mat screenshot_unchanged);
Mat binaryToMat(const char* data, int length);
string takeScreenshot();
string match(Mat photo, string result_dir);
