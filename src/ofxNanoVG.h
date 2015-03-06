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

	void setup(bool debug=false);

	void beginFrame(int width, int height, float devicePixelRatio);
	void endFrame();

	/******
	 * Shapes
	 */

	void drawRect(const ofRectangle& rect);
	void drawRect(float x, float y, float w, float h);

	void drawRoundedRect(const ofRectangle& rect, float r);
	void drawRoundedRect(float x, float y, float w, float h, float r);

	enum ofxNanoVGlineParam {
		NVG_BUTT,
		NVG_ROUND,
		NVG_SQUARE,
		NVG_BEVEL,
		NVG_MITER
	};
	// lineCap: one of: NVG_BUTT (default), NVG_ROUND, NVG_SQUARE
	// lineJoin: one of: NVG_MITER (default), NVG_ROUND, NVG_BEVEL
	void drawLine(float x1, float y1, float x2, float y2, enum ofxNanoVGlineParam cap=NVG_BUTT, enum ofxNanoVGlineParam join=NVG_MITER);
	void drawLine(const ofPoint& p1, const ofPoint& p2, enum ofxNanoVGlineParam cap=NVG_BUTT, enum ofxNanoVGlineParam join=NVG_MITER);
	void drawPolyline(const ofPolyline& line, enum ofxNanoVGlineParam cap=NVG_BUTT, enum ofxNanoVGlineParam join=NVG_MITER);


	/******
	 * Text
	 */




private:

	bool bInitialized;
	bool bInFrame;

	NVGcontext* ctx;

	// apply OF color and stroke width
	void applyOFStyle();

	// copy current OF matrix to nanovg
	void applyOFMatrix();

	// perform stroke or fill according to the current OF style.
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
