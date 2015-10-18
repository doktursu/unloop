#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxSlitScan.h"

class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

        /* Webcam */
		ofVideoGrabber      cam;
		ofPixels            camNormal;
		ofTexture           camTexture;
		int                 camWidth;
		int                 camHeight;
		unsigned char *     camPixels;

		/* ofxOpenCv */
		ofxCvColorImage     rgb;
		ofxCvGrayscaleImage grayImg, grayBg, grayDiff;
		unsigned char *     grayPixels;
		unsigned char *     diffPixels;

		/* Commands and parameters */
		bool                bLearnBackground;
		int                 threshold;

		/* ofxSlitScan */
		float               slitTimeWidth;
		ofxSlitScan         slitScan;
		ofxSlitScan         slitScan2;


		/* Timer */
		bool                bTimerReached;
		float               startTime;
		float               endTime;
		int                 timerCounter;

		/* Snap */
		bool                bSnap;
		int                 snapCounter;
		int                 imageIndex;
		vector<ofImage>    snapImages;

		/* ofCamera */
		ofCamera            ofCam;
		int                 ofCamPosition;
		float               decrement;

		/* Temporary */
		ofTexture           diffTexture;
		string              message;


};
