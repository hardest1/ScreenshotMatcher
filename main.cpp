// source: https://docs.opencv.org/3.4/d7/dff/tutorial_feature_homography.html
// or: https://github.com/opencv/opencv/blob/3.4/samples/cpp/tutorial_code/features2D/feature_homography/SURF_FLANN_matching_homography_Demo.cpp

#include <iostream>
#include "opencv2/core.hpp"
#ifdef HAVE_OPENCV_XFEATURES2D
#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"

using namespace cv;
using namespace cv::xfeatures2d;

int main( int argc, char* argv[] )
{
    // load the required images
    Mat img_object = imread( samples::findFile(argv[1]), IMREAD_GRAYSCALE );
    Mat img_scene = imread( samples::findFile(argv[2]), IMREAD_GRAYSCALE );
    Mat img_scene_colored = imread( samples::findFile(argv[2]), IMREAD_UNCHANGED );

    if ( img_object.empty() || img_scene.empty() )
    {
        printf("Could not open or find the image!\n");
        return -1;
    }

    //-- Step 1: Detect the keypoints using SURF Detector, compute the descriptors

    int minHessian = 400;
    Ptr<SURF> detector = SURF::create( minHessian );
    detector->setUpright(false); // might be faster when set to true

    std::vector<KeyPoint> keypoints_object, keypoints_scene;
    Mat descriptors_object, descriptors_scene;
    detector->detectAndCompute( img_object, noArray(), keypoints_object, descriptors_object );
    detector->detectAndCompute( img_scene, noArray(), keypoints_scene, descriptors_scene );

    //-- Step 2: Matching descriptor vectors with a FLANN based matcher

    // Since SURF is a floating-point descriptor NORM_L2 is used
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
    std::vector< std::vector<DMatch> > knn_matches;
    matcher->knnMatch( descriptors_object, descriptors_scene, knn_matches, 2 );

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

    // draw a border around the detected area
    line( img_scene_colored, scene_corners[0],
          scene_corners[1], Scalar(0, 255, 0), 4 );
    line( img_scene_colored, scene_corners[1],
          scene_corners[2], Scalar( 0, 255, 0), 4 );
    line( img_scene_colored, scene_corners[2],
          scene_corners[3], Scalar( 0, 255, 0), 4 );
    line( img_scene_colored, scene_corners[3],
          scene_corners[0], Scalar( 0, 255, 0), 4 );

    // crop the original screenshot to the detected area (no perspective transform!)
    Rect2d roi(minX, minY, maxX - minX, maxY - minY);
    roi = roi & Rect2d(0, 0, img_scene_colored.cols, img_scene_colored.rows);
    Mat img_crop = img_scene_colored(roi);

    imwrite("out.jpg", img_crop);

    return 0;
}
#else
int main()
{
    printf("This tutorial code needs the xfeatures2d contrib module to be run.\n");
    return 0;
}
#endif
