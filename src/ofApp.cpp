#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

void ofApp::setup() {
    ofSetVerticalSync(true);
    ofSetFrameRate(120);
    
    cam.setup(640, 480);
    buffer.assign(BUFFER_SIZE, 0);
}

void ofApp::update() {
    
    
    cam.update();
    if(cam.isFrameNew()) {
        Mat src, dst, gray;
        src = ofxCv::toCv(cam);
        
        // canny edge detection
        // http://docs.opencv.org/2.4/doc/tutorials/imgproc/imgtrans/canny_detector/canny_detector.html
        Canny(src, dst, 100, 300, 3);
        
        // find lines in image
        // http://docs.opencv.org/2.4/doc/tutorials/imgproc/imgtrans/hough_lines/hough_lines.html
        // lowering angle resolutin, see here http://stackoverflow.com/questions/21121674/more-accurate-houghline-opencv
        HoughLines(dst, lines, 1, 2*CV_PI / 180, 90);
        
        ofxCv::toOf(dst, cannyImg);
        
    }
}

void ofApp::draw() {
    ofSetColor(255, 255, 255);
    cam.draw(0, 0);
    
    ofPushMatrix();
    ofScale(0.3, 0.3);
    cannyImg.update();
    if (cannyImg.isAllocated()) {
        cannyImg.draw(0, 0);
    }
    ofPopMatrix();
    
    // draw in imaginary line, center of the screen
    int half = ofGetWidth()/2;
    int height = ofGetHeight();
    
    ofPoint ip1(half, 0);
    ofPoint ip2(half, height);
    
    ofSetColor(255, 0, 0);
    ofDrawLine(ip1, ip2);
    
    // draw the found lines
    int countLines = 0;
    for(size_t i = 0; i < lines.size(); i++) {
        float rho = lines[i][0];
        float theta = lines[i][1];
        double a = cos(theta), b = sin(theta);
        double x0 = a * rho, y0 = b * rho;
        
        cv::Point pt1(cvRound(x0 + 1000 * (-b)),
                      cvRound(y0 + 1000 * (a)));
        cv::Point pt2(cvRound(x0 - 1000 * (-b)),
                      cvRound(y0 - 1000 * (a)));
        
        int angle = abs(calcAngle(pt1, pt2));
        
        if((angle >= 80 && angle <= 100) ||
           (angle >= 170 && angle <= 190)) {
            
            ofPoint ofpt1 = ofxCv::toOf(pt1);
            ofPoint ofpt2 = ofxCv::toOf(pt2);
            
            ofSetColor(255, 255, 255);
            ofDrawLine(ofpt1, ofpt2);
            
            ofPoint intersect;
            ofLineSegmentIntersection(ofpt1, ofpt2, ip1, ip2, intersect);
            
            ofSetColor(0, 0, 255);
            ofDrawCircle(intersect.x, intersect.y, 5);
            
            countLines++;
        }
    }
    
    int foundLine = (countLines > 0) ? 1 : 0;
    buffer.push_front(foundLine);
    if (buffer.size() > BUFFER_SIZE) {
        buffer.pop_back();
    }
    
    int sum = 0;
    for (int el : buffer) {
        sum += el;
    }
    
    double mean = (double)sum / (double)buffer.size();
    cout << mean << endl;
    
    if (mean >= 0.8) {
        cout << "bam trigger arduino! (but only once in a window of x seconds)" << endl;
    }
    
}

int ofApp::calcAngle(cv::Point pt1, cv::Point pt2) {
    float deltaY = pt1.y - pt2.y;
    float deltaX = pt1.x - pt2.x;
    
    double angle = atan2(deltaY, deltaX) * 180.0 / PI;
    
    return (int) angle;
}

void ofApp::keyPressed() {
}