#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

void ofApp::setup() {
    // setup framerate and -sync
    ofSetVerticalSync(true);
    ofSetFrameRate(120);
    
    // setup our webcam resolution
    cam.setup(640, 480);
    
    // define the size of our buffer and prefill with 0
    buffer.assign(BUFFER_SIZE, 0);
    
    background.setLearningTime(2400);
    background.setThresholdValue(50);
}

void ofApp::update() {
    cam.update();
    if(cam.isFrameNew()) {
        Mat src, gray, blured, thresholded, masked, edges;
        // openframeworks image to opencv image
        src = ofxCv::toCv(cam);
        
        // convert color to gray image
        convertColor(src, gray, CV_BGR2GRAY);
        
        // Reduce noise with a kernel 3x3
        cv::blur(gray, blured, cv::Size(3,3));
        
        // create background substration mask
        background.update(blured, thresholded);
        
        // multiply background mask with our blured camera image
        cv::multiply(blured, thresholded, masked);
        
        // canny edge detection
        // http://docs.opencv.org/2.4/doc/tutorials/imgproc/imgtrans/canny_detector/canny_detector.html
        Canny(masked, edges, 100, 300, 3);
        
        // find lines in image
        // http://docs.opencv.org/2.4/doc/tutorials/imgproc/imgtrans/hough_lines/hough_lines.html
        // lowering angle resolutin, see here http://stackoverflow.com/questions/21121674/more-accurate-houghline-opencv
        HoughLines(edges, lines, 1, 2*CV_PI / 180, 85);
        
        // convert the opencv image back into an openframeworks image
        ofxCv::toOf(edges, cannyImg);
    }
}

void ofApp::draw() {
    // draw our camera image
    ofSetColor(255, 255, 255);
    cam.draw(0, 0);
    
    // draw the canny image (scaled down)
    ofPushMatrix();
    ofScale(0.3, 0.3);
    cannyImg.update();
    if (cannyImg.isAllocated()) {
        cannyImg.draw(0, 0);
    }
    ofPopMatrix();
    
    // draw (vertical) imaginary line, center of the screen
    int half = ofGetWidth()/2;
    int height = ofGetHeight();
    
    // this is our start and endpoint of the (vertical) "intersection" line
    ofPoint ip1(half, 0);
    ofPoint ip2(half, height);
    
    // draw our intersection line
    ofSetColor(255, 0, 0);
    ofDrawLine(ip1, ip2);
    
    // store intersections in here
    std::vector<ofPoint> intersections;
    
    // handle the found hough lines (we are interating over them, one by one)
    for(size_t i = 0; i < lines.size(); i++) {
        // do some calculations to get back the points from opencv
        float rho = lines[i][0];
        float theta = lines[i][1];
        double a = cos(theta), b = sin(theta);
        double x0 = a * rho, y0 = b * rho;
        
        // here we calculate the start- and endpoint of our line (one by the time!)
        cv::Point pt1(cvRound(x0 + 1000 * (-b)),
                      cvRound(y0 + 1000 * (a)));
        cv::Point pt2(cvRound(x0 - 1000 * (-b)),
                      cvRound(y0 - 1000 * (a)));
        
        // calculate the absolute angle of our line
        int angle = abs(calcAngle(pt1, pt2));
        
        // only take the ones that are either perpendicular or vertical aligned
        // TODO: split between horizontal and vertical lines and then also do the
        //       intersection calc with an imagenary horizontal line!
        if((angle >= 80 && angle <= 100) ||
           (angle >= 170 && angle <= 190)) {
            
            // convert opencv point to eopenframeworks point structure,
            // so we can use ofLineSegmentIntersection
            ofPoint ofpt1 = ofxCv::toOf(pt1);
            ofPoint ofpt2 = ofxCv::toOf(pt2);
            
            // calculate ourintersection
            ofPoint intersect;
            ofLineSegmentIntersection(ofpt1, ofpt2, ip1, ip2, intersect);
            
            // store the intersection outside of the for loop
            intersections.push_back(intersect);
            
            // draw our found (hough) line
            ofSetColor(255, 255, 255);
            ofDrawLine(ofpt1, ofpt2);
            
            // draw our intersection point
            ofSetColor(0, 0, 255);
            ofDrawCircle(intersect.x, intersect.y, 5);
        }
    }
    
    // interate over our intersection points and calculate the distance to each other
    int countLines = 0;
    for (int i=0; i < intersections.size(); i++) {
        ofPoint intersection = intersections[i];
        // if we have a least 2 points we can start to check their distance
        if (i > 0) {
            ofPoint prevIntersection = intersections[i-1]; // see i - 1
            float distance = intersection.distance(prevIntersection);
            // here we check if the distance is in a certain range (width of our phone)
            // TODO: find the right distance values
            if (distance > MIN_DISTANCE && distance < MAX_DISTANCE) {
                countLines++;
            }
        }
    }
    
    // check if we have at least two intersection points (one line) in a certain distance
    int foundLine = (countLines > 0) ? 1 : 0;
    // then we fill our buffer with the result
    buffer.push_front(foundLine);
    
    // sliding buffer ;)
    if (buffer.size() > BUFFER_SIZE) {
        buffer.pop_back();
    }
    
    // calculation sum and avg
    int sum = 0;
    for (int el : buffer) {
        sum += el;
    }
    double avg = (double)sum / (double)buffer.size();
    cout << avg << endl;
    
    // trigger something if we reach the threshold
    // TODO: find the right threshold value
    if (avg >= THRESHOLD) {
        cout << "bam trigger arduino! (but only once in a window of x seconds)" << endl;
        // TODO: implement a timer that suspends the detection for a certain time after triggered once
    }
    
}

int ofApp::calcAngle(cv::Point pt1, cv::Point pt2) {
    float deltaY = pt1.y - pt2.y;
    float deltaX = pt1.x - pt2.x;
    
    double angle = atan2(deltaY, deltaX) * 180.0 / PI;
    
    return (int) angle;
}

void ofApp::keyPressed(int key) {
    background.reset();
}