#include "ofApp.h"

#define DISPLAY_WIDTH 1024
#define DISPLAY_HEIGHT 768

#define NUM_CHANNELS_RGBA 4
#define NUM_CHANNELS_RGB 3

#define INITIAL_OFCAMPOSITION 370
#define MAX_SNAPS 42
#define DRAW_Y 60 // 60 Y position which all elements are drawn
#define DRAW_Z 11100 //11100

//--------------------------------------------------------------
void ofApp::setup(){

    /* Webcam */
    camWidth = 1024; //1024,320
    camHeight = 576; //576,240

    cam.setDeviceID(0);
    cam.setPixelFormat(OF_PIXELS_RGB);
    cam.initGrabber(camWidth, camHeight);

    camPixels = cam.getPixels();

    camTexture.allocate(camWidth, camHeight, GL_RGB);


    /* ofxOpenCv */
    rgb.allocate(camWidth, camHeight);
    grayImg.allocate(camWidth, camHeight);
    grayBg.allocate(camWidth, camHeight);
    grayDiff.allocate(camWidth, camHeight);

    diffPixels = new unsigned char[camWidth*camHeight*NUM_CHANNELS_RGBA];


    /* ofxSlitScan */
    slitTimeWidth = 200.0;

    int capacity = 200;
    slitScan.setup(camWidth, camHeight, capacity);
    slitScan.setTimeDelayAndWidth(0, slitTimeWidth);
    /* Second slitScan image with short capacity and time width of 2 */
    slitScan2.setup(camWidth, camHeight, 2);
    slitScan2.setTimeDelayAndWidth(0, 2);


    /* Commands and parameters*/
    bLearnBackground = false;
    threshold = 30;


    /* Timer */
    bTimerReached = false;
    startTime = ofGetElapsedTimeMillis(); // Set startTime to beginning time of program
    endTime = 1000; // First snap at 1 second
    timerCounter = 0;

    /* Snap */
    bSnap = true; // Begin snapping at program start
    snapCounter = 0;
    imageIndex = 0;
    snapImages.assign(42, ofImage());

    /* ofCamera */
    ofCamPosition = INITIAL_OFCAMPOSITION;
    decrement = 0.1;

    /* Temporary */
    diffTexture.allocate(camWidth, camHeight, GL_RGBA);

    int nChannels = cam.getPixelsRef().getNumChannels();
    message = "Number of Channels: " + ofToString(nChannels);


}

//--------------------------------------------------------------
void ofApp::update(){

    /* ofCamera setup */
    ofCam.setPosition(ofPoint(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2, DRAW_Z + ofCamPosition));
    ofCam.lookAt(ofVec3f(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2, -2800-70)); // -2800-70

    /* Set program background to black */
    ofBackgroundHex(0x000000);

    /* Update Webcam and set camTexture to webcam input */
    cam.update();
    camTexture = cam.getTextureReference(); // OR: camTexture.loadData(cam.getPixels());

    /* ofxOpenCv */
    if (cam.isFrameNew()) {
        rgb.setFromPixels(cam.getPixels(), camWidth, camHeight);

        /* '=' copies the pixels from one ofxCvImage to another (operator overloading) */
        grayImg = rgb;

        /* Set background image with key ' ' */
        if (bLearnBackground) {
            grayBg = grayImg;
            bLearnBackground = false;
        }

        /* Find absDiff between background and incoming image and then threshold */
        grayDiff.absDiff(grayBg, grayImg);
        grayDiff.threshold(threshold);

        /* Set grayPixels from grayDiff pixels*/
        grayPixels = grayDiff.getPixels();

        /*
         * Keep different pixels as is,
         * and replace same pixels with transparent pixels (alpha=0)
         * 0 is Black/Transparent, 255 is White/Opaque
         */
         for (int i = 0; i < grayImg.getPixels().size(); i++) {
            if (grayPixels[i] == 255) {
                diffPixels[i*NUM_CHANNELS_RGBA] = camPixels[i*NUM_CHANNELS_RGB];
                diffPixels[i*NUM_CHANNELS_RGBA+1] = camPixels[i*NUM_CHANNELS_RGB+1];
                diffPixels[i*NUM_CHANNELS_RGBA+2] = camPixels[i*NUM_CHANNELS_RGB+2];
                diffPixels[i*NUM_CHANNELS_RGBA+3] = 255;
            } else {
                diffPixels[i*NUM_CHANNELS_RGBA] = 0;
                diffPixels[i*NUM_CHANNELS_RGBA+1] = 0;
                diffPixels[i*NUM_CHANNELS_RGBA+2] = 0;
                diffPixels[i*NUM_CHANNELS_RGBA+3] = 0;
            }
         }

         /* Add diffPixels to slitScans */
         slitScan.addImage(diffPixels);
         slitScan2.addImage(diffPixels);

        /* Temporary */
        diffTexture.loadData(diffPixels, camWidth, camHeight, GL_RGBA);
    }

    /* Update slitScan time width to match changes in draw() */
    slitScan.setTimeDelayAndWidth(0, slitTimeWidth);
}

//--------------------------------------------------------------
void ofApp::draw(){

    /* ofCamera */
    ofCam.begin();

    /* When snapCounter reaches max: */
    if (snapCounter == MAX_SNAPS) {
        /* Stop snapping */
        bSnap = false;
        /* Draw snaps in 3D space */
        for (int i = 0; i < MAX_SNAPS-2; i++) {
            snapImages[i].draw(0, DRAW_Y+15, -23800+i*700);
        }

        /* Draw final snap (from cam) spaced further apart and closer to ofCam */
        /* closer by 650 */
        snapImages[MAX_SNAPS-1].draw(0, DRAW_Y+15, 4500);

        /* Move ofCam forward through the snaps, increasing the speed(decrement) */
        decrement += 0.08;
        ofCamPosition -= decrement;

        if (ofCamPosition < -23800-8000) {
            /* Reset Snaps */
            snapCounter = 0; // This also stops ofCam sequence
            bSnap = true;
            imageIndex = 0;
            endTime = 1000;
            timerCounter = 0;

            /* Reset ofCamera */
            ofCamPosition = INITIAL_OFCAMPOSITION;
            decrement = 0.1;

            /* Reset slitTimeWidth */
            slitTimeWidth = 200.0;
        }

    }

    //cam.draw(20,20);
    camTexture.draw(0, DRAW_Y, DRAW_Z);
    slitScan.getOutputImage().draw(0, DRAW_Y, DRAW_Z);

    /* Timer to takes snaps */
    float timer = ofGetElapsedTimeMillis() - startTime;

    if (timer >= endTime && !bTimerReached) {
        bTimerReached = true;
        /* Update startTime to current time */
        startTime = ofGetElapsedTimeMillis();

        /* After initial snap, extend endTime to 2 seconds to slow down process */
        if (timerCounter == 1) endTime = 2000;

        /* After 10 snaps, extend endTime to 5 seconds to further slow down process */
        if (timerCounter == 10) endTime = 5000;

        timerCounter++;

        /* Reset timerCounter when sequence reaches max snaps */
        if (timerCounter == 39) timerCounter = 0;
    }

    /* Draw slitScan2 for first few seconds of timer */
    if (timerCounter < 10) {
        slitScan2.getOutputImage().draw(0, DRAW_Y, DRAW_Z);
    }

    /* ofxSlitScan:
     * After 20 snaps, slowly decrease slitTimeWidth so slitScan image catches up to cam input.
     * When slitTimeWidth reaches 3, keep at 3.
     */
    if (snapCounter >20) {
        slitTimeWidth -= 0.05;
        if (slitTimeWidth < 3) slitTimeWidth = 3;
    }

    /* Snap */
    if (bTimerReached && bSnap) {
        /* First 30 snaps from diffPixels/slitScan, without alpha for glitch effect */
        if (snapCounter < 31)
            snapImages[imageIndex].setFromPixels(diffPixels, camWidth, camHeight, OF_IMAGE_COLOR);
        /* Last 10 snaps from diffPixels/slitScan, with alpha */
        if (snapCounter >= 31 && snapCounter < 40)
            snapImages[imageIndex].setFromPixels(diffPixels, camWidth, camHeight, OF_IMAGE_COLOR_ALPHA);
        /* Last snap from cam, normal */
        if (snapCounter >= 41)
            snapImages[imageIndex].setFromPixels(camPixels, camWidth, camHeight, OF_IMAGE_COLOR);
        imageIndex++;
        snapCounter++;

        bTimerReached = false;
    }

    /* Temporary */
    //diffTexture.draw(20 + camWidth*2, 20, 50);
    /*
    ofDrawBitmapString(message, 20, 40 +camHeight);
    string type = "Image type: " + ofToString(slitScan.getType());
    ofDrawBitmapString(type, 20, 50 +camHeight);
    */
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    switch (key) {
        case ' ':
            bLearnBackground = true;
            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}
