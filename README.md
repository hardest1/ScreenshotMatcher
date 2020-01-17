# ScreenshotMatcher

Proof of concept program to match photographs of computer screens with actual screenshots.
Heavily based on OpenCV sample code for planar homography [1, 2].

# Usage

OpenCV with contrib packages is required as the non-free SURF algorithm is used.

You might have to adjust the include path in the Makefile.

Execute the compiled program with parameters for an actual screenshot and the photograph of a subsection of this screenshot (scaled down to 512 px width seems to work better than original size): 

```./ScreenshotMatcher screenshot1_small.jpg org1.png```

A new file (out.jpg) containing the cropped region of the screenshot will be created.

# References

[1] https://docs.opencv.org/3.4/d7/dff/tutorial_feature_homography.html

[2] https://github.com/opencv/opencv/blob/3.4/samples/cpp/tutorial_code/features2D/feature_homography/SURF_FLANN_matching_homography_Demo.cpp

