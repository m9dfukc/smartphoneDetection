#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

//#include "ofxCv/Tracker.h"
//#include "ofxCv/Utilities.h"
//#include "ofRectangle.h"
//#include "ofVec2f.h"
//
//namespace ofxCv {
//    float trackingDistance(const cv::Rect& a, const cv::Rect& b) {
//        float dx = (a.x + a.width / 2.) - (b.x + b.width / 2.);
//        float dy = (a.y + a.height / 2.) - (b.y + b.height / 2.);
//        float dw = a.width - b.width;
//        float dh = a.height - b.height;
//        float pd = sqrtf(dx * dx + dy * dy);
//        float sd = sqrtf(dw * dw + dh * dh);
//        return pd + sd;
//    }
//
//    float trackingDistance(const cv::Point2f& a, const cv::Point2f& b) {
//        float dx = a.x - b.x;
//        float dy = a.y - b.y;
//        return sqrtf(dx * dx + dy * dy);
//    }
//
//    float trackingDistance(const ofRectangle& a, const ofRectangle& b) {
//        return trackingDistance(toCv(a), toCv(b));
//    }
//
//    float trackingDistance(const ofVec2f& a, const ofVec2f& b) {
//        return trackingDistance(toCv(a), toCv(b));
//    }
//}

void ofApp::setup() {
    ofSetVerticalSync(true);
    ofSetFrameRate(120);

    //finder.setup("haarcascade_nexus5.xml");
    //finder.setup("haarcascade_iphone_macbook-pro.xml");
    finder.setup("haarcascade_frontalface_default.xml");
    finder.setPreset(ObjectFinder::Fast);

    cam.setup(640, 480);
    background.setLearningTime(1000);
    background.setThresholdValue(50);
}

void ofApp::update() {
    cam.update();
    if(cam.isFrameNew()) {
        finder.update(cam);

        background.update(cam, thresholded);

        Mat src, dst, gray;
        src = ofxCv::toCv(cam);

        // canny edge detection
        // http://docs.opencv.org/2.4/doc/tutorials/imgproc/imgtrans/canny_detector/canny_detector.html
        Canny(src, dst, 100, 300, 3);

        // create cannyImg
        ofxCv::toOf(dst, cannyImg);

        // convert to grayscale
        // http://docs.opencv.org/2.4/modules/imgproc/doc/miscellaneous_transformations.html#cvtcolor
        cvtColor(src, gray, CV_BGR2GRAY);

        // find lines in image
        // http://docs.opencv.org/2.4/doc/tutorials/imgproc/imgtrans/hough_lines/hough_lines.html
        HoughLines(dst, lines, 1, CV_PI / 180, 100);
        //HoughLinesP(dst, lines, 1, CV_PI / 180, 50, 50, 10);

        // reduce noise to avoid false circle detection
        // http://docs.opencv.org/2.4/modules/imgproc/doc/filtering.html#gaussianblur
        //GaussianBlur(gray, gray, cv::Size(3, 3), 2, 2);

        // find circles in image
        // http://docs.opencv.org/2.4/doc/tutorials/imgproc/imgtrans/hough_circle/hough_circle.html
        //HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 1, gray.rows / 10, 55, 30, 0, 40);
    }
}

void ofApp::draw() {
    cam.draw(0, 0);
    finder.draw();
    ofDrawBitmapStringHighlight(ofToString(finder.size()), 10, 20);

    // draw background
    ofPushMatrix();
    ofTranslate(160, 0);
    ofScale(0.25, 0.25);
    thresholded.update();
    thresholded.draw(0, 0);
    ofPopMatrix();

    // draw cannyImg
    ofPushMatrix();
    ofScale(0.25, 0.25);
    cannyImg.update();
    cannyImg.draw(0, 0);
    ofPopMatrix();

    // draw HoughLines
    for(size_t i = 0; i < lines.size(); i++) {
        float rho = lines[i][0];
        float theta = lines[i][1];
        double a = cos(theta), b = sin(theta);
        double x0 = a * rho, y0 = b * rho;

        cv::Point pt1(cvRound(x0 + 1000 * (-b)),
                      cvRound(y0 + 1000 * (a)));
        cv::Point pt2(cvRound(x0 - 1000 * (-b)),
                      cvRound(y0 - 1000 * (a)));

//        cout << "line " << i << ":" << endl;
//        cout << calcAngle(pt1, pt2) << endl;

        if((calcAngle(pt1, pt2) >= -5 && calcAngle(pt1, pt2) <= 5) || (calcAngle(pt1, pt2) >= 85 && calcAngle(pt1, pt2) <= 95)) {
            ofSetColor(255, 255, 255);
            ofDrawLine(pt1.x, pt1.y, pt2.x, pt2.y);
        }
    }

//    // draw HoughLinesP
//    for(size_t i = 0; i < lines.size(); i++) {
//        cv::Vec4i l = lines[i];
//
//        ofSetColor(255, 255, 255);
//        ofDrawLine(l[0], l[1], l[2], l[3]);
//    }

//    // draw HoughCirles
//    for(size_t i = 0; i < circles.size(); i++) {
//        ofVec2f center(circles[i][0], circles[i][1]);
//        int radius = circles[i][2];
//        cout << radius << endl;
//        ofSetColor(255, 255, 255);
//        ofDrawEllipse(center.x, center.y, radius , radius);
//    }
}

void ofApp::keyPressed(int key) {
    if(key == ' ') {
        background.reset();
    }
}

int ofApp::calcAngle(cv::Point pt1, cv::Point pt2) {
    float deltaX = pt2.x - pt1.x;
    float deltaY = pt2.y - pt1.y;

    double angle = atan2(deltaY, deltaX) * 180 / CV_PI;

    return (int) angle;
}