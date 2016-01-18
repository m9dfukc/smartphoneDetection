#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"

#define BUFFER_SIZE 60
#define THRESHOLD 0.85
#define MIN_DISTANCE 30
#define MAX_DISTANCE 250


class ofApp : public ofBaseApp {
    
public:
    void setup();
    void update();
    void draw();
    void keyPressed(int key);
    
    int calcAngle(cv::Point pt1, cv::Point pt2);
    
    vector<cv::Vec2f> lines;
    std::deque<int> buffer;
    
    ofImage cannyImg;
    ofVideoGrabber cam;
    ofxCv::RunningBackground background;
};