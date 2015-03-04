//
//  ofxNanoVG.h
//  sentopiary
//
//  Created by Gal Sasson on 12/27/14.
//
//

#ifndef __sentopiary__ofxNanoVG__
#define __sentopiary__ofxNanoVG__

#include <stdio.h>
#include "ofMain.h"

class NVGcontext;

class ofxNanoVG
{
public:
	~ofxNanoVG();

	static ofxNanoVG& one()
	{
		static ofxNanoVG instance;
		// Instantiated on first use.
		return instance;
	}

	void setup();

	void beginFrame(int width, int height, float devicePixelRatio);
	void endFrame();

	void drawRoundedRect(float x, float y, float w, float h, float r);
	void drawPolyline(const ofPolyline& line);

private:

	bool bInitialized;
	bool bInFrame;

	NVGcontext* ctx;

	void setOFStyle();
	void doOFDraw();

	ofxNanoVG() :
		bInitialized(false),
		bInFrame(false),
		ctx(NULL) {}

	// make sure there are no copies
	ofxNanoVG(ofxNanoVG const&);
	void operator=(ofxNanoVG const&);


};

#endif /* defined(__sentopiary__ofxNanoVG__) */
