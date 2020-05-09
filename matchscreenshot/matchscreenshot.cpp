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

Mat matchScreenshot(Mat photo, Mat screenshot, Mat screenshot_unchanged, int algo, int _n)
{
    DescriptorMatcher::MatcherType _matcher;
    std::vector<KeyPoint> keypoints_object, keypoints_scene;
    Mat descriptors_object, descriptors_scene;

    Mat img_object = photo;
    Mat img_scene = screenshot;
    Mat img_scene_colored = screenshot_unchanged;

    Mat nullmat;

    if ( img_object.empty() || img_scene.empty() )
    {
        return nullmat;
    }

    // 1 = ORB (default)
    // 2 = SURF
    // 3 = SIFT

    switch (algo){
        case 2:
        {
            // SURF
            Ptr<SURF> detector = SURF::create( _n );
            detector->setUpright(true);
            _matcher = DescriptorMatcher::FLANNBASED;
            detector->detectAndCompute( img_object, noArray(), keypoints_object, descriptors_object );
            detector->detectAndCompute( img_scene, noArray(), keypoints_scene, descriptors_scene );
            break;
        }
        case 3:
        {
            // SIFT
            Ptr<SIFT> detector = SIFT::create( _n );
            _matcher = DescriptorMatcher::FLANNBASED;
            detector->detectAndCompute( img_object, noArray(), keypoints_object, descriptors_object );
            detector->detectAndCompute( img_scene, noArray(), keypoints_scene, descriptors_scene );
            break;
        }
        default:
        {
            // ORB
            Ptr<ORB> detector = ORB::create( _n );
            _matcher = DescriptorMatcher::BRUTEFORCE_HAMMING;
            detector->detectAndCompute( img_object, noArray(), keypoints_object, descriptors_object );
            detector->detectAndCompute( img_scene, noArray(), keypoints_scene, descriptors_scene );
        }
    }

    //-- Step 2: Matching descriptor vectors

    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(_matcher);
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

    cout << "Filtered/Good matches: " << good_matches.size() << endl;

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
    Mat H = findHomography( obj, scene, RANSAC );
    //Mat H = findHomography( obj, scene, LMEDS );


    if ( H.empty() )
    {   
        cout << "No Result" << endl;
        return nullmat;
    }

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

    // crop the original screenshot to the detected area (no perspective transform!)
    Rect2d roi(minX, minY, maxX - minX, maxY - minY);
    roi = roi & Rect2d(0, 0, img_scene_colored.cols, img_scene_colored.rows);
    Mat img_crop = img_scene_colored(roi);

    cout << ( img_crop.empty() ? "No Result" : "Success" ) << endl;
    
    return img_crop;
}

string test_algos(int image, int algo, int n, string scriptDir)
{
    Mat out;

    string photoPath = "/test_data/test_suite/photo";
    string screenPath = "/test_data/test_suite/screenshot";

    Mat photo = imread( samples::findFile(scriptDir + photoPath + to_string(image) + ".jpg"), IMREAD_GRAYSCALE );
    Mat screen = imread( samples::findFile(scriptDir + screenPath + to_string(image) + ".png"), IMREAD_GRAYSCALE );
    Mat screen_unchanged = imread( samples::findFile(scriptDir + screenPath + to_string(image) + ".png"), IMREAD_UNCHANGED );

    out = matchScreenshot(photo, screen, screen_unchanged, algo, n);

    if(out.empty()) { return "no result"; }

    string filename = "test-" + to_string(time(0)) + "-" + generate_hex(5) + ".jpg";

    string out_path = scriptDir + "/www/results/tests/" + filename;

    imwrite( out_path, out);
    
    return filename;
    
}

tuple<string, bool, string> match(const char* data, int length, string result_dir)
{
    string uid = generate_hex(16);


    cout << "Taking screenshot.. " << endl;

    string screenshot_path = takeScreenshot(uid);

    cout << "Reading images.. " << endl;

    std::vector<unsigned char> ImVec(data, data + length);
    Mat photo = imdecode(ImVec, IMREAD_GRAYSCALE);
    Mat photo_unchanged = imdecode(ImVec, IMREAD_UNCHANGED);

    Mat screen = imread( screenshot_path.c_str(), IMREAD_GRAYSCALE );
    Mat screen_unchanged = imread( screenshot_path.c_str(), IMREAD_UNCHANGED );
    
    // Save photo for possible feedback submission
    imwrite("/tmp/photo-" + uid + ".jpg", photo_unchanged);

    cout << "Match algorithm START" << endl;

    Mat out = matchScreenshot(photo, screen, screen_unchanged, 1, 800);

    cout << "Match algorithm END" << endl;

    if(out.empty()) { 
        return { uid, false, "" };
    }

    string filename = "result-" + uid + ".jpg";

    string out_path = result_dir + "/" + filename;

    imwrite( out_path, out);

    return { uid, true, filename };
    
}

string takeScreenshot(string uid)
{
    ScreenShot screen(0);
    Mat img;
    screen(img);
    string filename = "/tmp/screenshot-" + uid + ".jpg";
    imwrite(filename.c_str(), img);
    return filename;
}

// Source for random_char and generate_hex: https://lowrey.me/guid-generation-in-c-11/

unsigned int random_char() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    return dis(gen);
}

std::string generate_hex(const unsigned int len) {
    std::stringstream ss;
    for (auto i = 0; i < len; i++) {
        const auto rc = random_char();
        std::stringstream hexstream;
        hexstream << std::hex << rc;
        auto hex = hexstream.str();
        ss << (hex.length() < 2 ? '0' + hex : hex);
    }
    return ss.str();
}