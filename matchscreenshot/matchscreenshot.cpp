#include "matchscreenshot.hpp"

struct ScreenShot
{
    ScreenShot(int x):
        x(x)
    {
        display = XOpenDisplay(nullptr);
        root = DefaultRootWindow(display);
        s = DefaultScreenOfDisplay(display);

        init = true;
    }

    void operator() (Mat& cvImg)
    {
        if(init == true)
            init = false;
        else
            XDestroyImage(img);

        img = XGetImage(display, root, 0, 0, s->width, s->height, AllPlanes, ZPixmap);
        cvImg = Mat(s->height, s->width, CV_8UC4, img->data);
    }

    ~ScreenShot()
    {
        if(init == false)
            XDestroyImage(img);

        XCloseDisplay(display);
    }

    Display* display;
    Screen*  s;
    Window root;
    int x, y;
    XImage* img;

    bool init;
};

Mat matchScreenshot(Mat photo, Mat screenshot, Mat screenshot_unchanged)
{

    Mat img_object = photo; //imread( samples::findFile(path_photo), IMREAD_GRAYSCALE );
    Mat img_scene = screenshot; //imread( samples::findFile(path_screen), IMREAD_GRAYSCALE );
    Mat img_scene_colored = screenshot_unchanged; //imread( samples::findFile(path_screen), IMREAD_UNCHANGED );

    Mat nullmat;

    if ( img_object.empty() || img_scene.empty() )
    {
        return nullmat;
    }

    //-- Step 1: Detect the keypoints using SURF Detector, compute the descriptors

    int minHessian = 400;
    Ptr<SURF> detector = SURF::create( minHessian );
    detector->setUpright(false); // might be faster when set to true

    cout << "Set SURF detector upright" << endl;

    std::vector<KeyPoint> keypoints_object, keypoints_scene;
    Mat descriptors_object, descriptors_scene;
    detector->detectAndCompute( img_object, noArray(), keypoints_object, descriptors_object );
    detector->detectAndCompute( img_scene, noArray(), keypoints_scene, descriptors_scene );

    cout << "Detected keypoints" << endl;

    //-- Step 2: Matching descriptor vectors with a FLANN based matcher

    // Since SURF is a floating-point descriptor NORM_L2 is used
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
    std::vector< std::vector<DMatch> > knn_matches;
    matcher->knnMatch( descriptors_object, descriptors_scene, knn_matches, 2 );

    cout << "knn Matches: " << knn_matches.size() << endl;

    //-- Filter matches using the Lowe's ratio test
    const float ratio_thresh = 0.75f;
    std::vector<DMatch> good_matches;
    for (size_t i = 0; i < knn_matches.size(); i++)
    {
        if (knn_matches[i][0].distance < ratio_thresh * knn_matches[i][1].distance)
        {
            good_matches.push_back(knn_matches[i][0]);
        }
    }

    cout << "Filtered matches" << endl;

    //-- Localize the object
    std::vector<Point2f> obj;
    std::vector<Point2f> scene;
    for( size_t i = 0; i < good_matches.size(); i++ )
    {
        //-- Get the keypoints from the good matches
        obj.push_back( keypoints_object[ good_matches[i].queryIdx ].pt );
        scene.push_back( keypoints_scene[ good_matches[i].trainIdx ].pt );
    }

    // not sure which algorithm is better ¯\_(ツ)_/¯
    //Mat H = findHomography( obj, scene, RANSAC );
    Mat H = findHomography( obj, scene, LMEDS );

    cout << "Found homography" << endl;

    //-- Get the corners from the image_1 ( the object to be "detected" )
    std::vector<Point2f> obj_corners(4);
    obj_corners[0] = Point2f(0, 0);
    obj_corners[1] = Point2f( (float)img_object.cols, 0 );
    obj_corners[2] = Point2f( (float)img_object.cols, (float)img_object.rows );
    obj_corners[3] = Point2f( 0, (float)img_object.rows );
    std::vector<Point2f> scene_corners(4);
    perspectiveTransform( obj_corners, scene_corners, H);

    // find a bounding box around the detected area (used for cropping)
    float minX = scene_corners[0].x;
    float minY = scene_corners[0].y;
    float maxX = scene_corners[0].x;
    float maxY = scene_corners[0].y;

    for(int i = 1; i < 4; i++)
    {
        if(scene_corners[i].x < minX) minX = scene_corners[i].x; 
        if(scene_corners[i].x > maxX) maxX = scene_corners[i].x; 
        if(scene_corners[i].y < minY) minY = scene_corners[i].y; 
        if(scene_corners[i].y > maxY) maxY = scene_corners[i].y; 
    }
    /*
    // draw a border around the detected area
    line( img_scene_colored, scene_corners[0],
          scene_corners[1], Scalar(0, 255, 0), 4 );
    line( img_scene_colored, scene_corners[1],
          scene_corners[2], Scalar( 0, 255, 0), 4 );
    line( img_scene_colored, scene_corners[2],
          scene_corners[3], Scalar( 0, 255, 0), 4 );
    line( img_scene_colored, scene_corners[3],
          scene_corners[0], Scalar( 0, 255, 0), 4 );
    */

    cout << "Created box around object" << endl;

    // crop the original screenshot to the detected area (no perspective transform!)
    Rect2d roi(minX, minY, maxX - minX, maxY - minY);
    roi = roi & Rect2d(0, 0, img_scene_colored.cols, img_scene_colored.rows);
    Mat img_crop = img_scene_colored(roi);

    if(img_crop.empty())
    {
        cout << "No result" << endl;
    }
    else
    {
        cout << "Success" << endl;
    }
    


    return img_crop;
}

string match(Mat photo, string result_dir)
{

    cout << "Taking screenshot.. ";

    string screenshot_path = takeScreenshot();

    cout << "Done" << endl;

    cout << "Reading images.. ";

    Mat screen = imread( screenshot_path.c_str(), IMREAD_GRAYSCALE );
    Mat screen_unchanged = imread( screenshot_path.c_str(), IMREAD_UNCHANGED );

    cout << "Done" << endl;

    cout << "-- Match algorithm --" << endl;

    Mat out = matchScreenshot(photo, screen, screen_unchanged);

    // DEBUG
    /*
    Mat photo_test = imread( samples::findFile("test_data/photo.jpg"), IMREAD_GRAYSCALE );
    Mat screen_test = imread( samples::findFile("test_data/screenshot.png"), IMREAD_GRAYSCALE );
    Mat screen_unchanged_test = imread( samples::findFile("test_data/screenshot.png"), IMREAD_UNCHANGED );
    Mat out = matchScreenshot(photo_test, screen_test, screen_unchanged_test);
    */



    if(out.empty())
    {
        return "no result";
    }

    time_t t = time(0);

    string filename = to_string(t) + ".jpg";

    string out_path = result_dir + "/" + filename;

    imwrite( out_path, out);
    
    return filename;
    
}

Mat binaryToMat(const char* data, int length)
{
    std::vector<unsigned char> ImVec(data, data + length);
    Mat img = imdecode(ImVec, IMREAD_GRAYSCALE);
    //Mat img2 = imdecode(ImVec, IMREAD_UNCHANGED);
    //imwrite("test_data/cr7.jpg", img2);
    return img;
}

string takeScreenshot()
{
    ScreenShot screen(0);
    Mat img;
    screen(img);
    time_t t = time(0);
    string filename = "/tmp/screenshot-" + to_string(t) + ".jpg";
    imwrite(filename.c_str(), img);
    return filename;
}

