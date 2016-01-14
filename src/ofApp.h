#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"

class ofApp : public ofBaseApp {

public:
    void setup();
    void update();
    void draw();
    void keyPressed(int key);

    int calcAngle(cv::Point pt1, cv::Point pt2);

    vector<cv::Vec2f> lines;
    //vector<cv::Vec4i> lines;
    //vector<cv::Vec3f> circles;

    ofImage cannyImg;
    ofVideoGrabber cam;
    ofxCv::ObjectFinder finder;

    ofxCv::RunningBackground background;
    ofImage thresholded;
};