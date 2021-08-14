#include "utils.h"
#include <opencv2/opencv.hpp>

// void simgPadding(Mat img, Mat &imgRes, int minEdgeLimit, int maxEdgeLimit) {

//     double resizeRatio = maxEdgeLimit * 1.0 / max(img.rows,img.cols);
//     resize(img,imgRes,Size(resizeRatio*img.cols,img.rows*resizeRatio));
//     colResizeRatio = resizeRatio;
//     rowResizeRatio = resizeRatio;
//     Mat imgEdgeLimit(minEdgeLimit,maxEdgeLimit,img.type(),Scalar(0,0,0));
//     imgRes.copyTo(imgEdgeLimit(Rect(0,0,imgRes.cols,imgRes.rows)));
//     imgRes = imgEdgeLimit.clone();
// }