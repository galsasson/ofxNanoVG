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
	void flush();

	/******
	 * Shapes
	 */

	enum LineParam {
		NVG_BUTT,
		NVG_ROUND,
		NVG_SQUARE,
		NVG_BEVEL,
		NVG_MITER
	};
	// lineCap: one of: NVG_BUTT (default), NVG_ROUND, NVG_SQUARE
	// lineJoin: one of: NVG_MITER (default), NVG_ROUND, NVG_BEVEL

	void drawRect(float x, float y, float w, float h);

	void drawRect(const ofRectangle& rect) {
		drawRect(rect.x, rect.y, rect.width, rect.height);
	}

	void drawRoundedRect(float x, float y, float w, float h, float r);

	void drawRoundedRect(const ofRectangle& rect, float r) {
		drawRoundedRect(rect.x, rect.y, rect.width, rect.height, r);
	}

	void drawEllipse(float cx, float cy, float rx, float ry);

	void drawCircle(const ofPoint& p, float r) {
		drawCircle(p.x, p.y, r);
	}

	void drawCircle(float cx, float cy, float r);

	void drawLine(float x1, float y1, float x2, float y2,
				  enum LineParam cap=NVG_BUTT, enum LineParam join=NVG_MITER);

	void drawLine(const ofPoint& p1, const ofPoint& p2,
				  enum LineParam cap=NVG_BUTT, enum LineParam join=NVG_MITER);

	void drawPolyline(const ofPolyline& line,
					  enum LineParam cap=NVG_BUTT, enum LineParam join=NVG_MITER);


	/******
	 * Text
	 */

	struct Font {
		int id;
		string name;
		float letterSpacing;
		float lineHeight;
	};

	enum TextHorizontalAlign {
		// Horizontal align
		NVG_ALIGN_LEFT 		= 1<<0,	// Default, align text horizontally to left.
		NVG_ALIGN_CENTER 	= 1<<1,	// Align text horizontally to center.
		NVG_ALIGN_RIGHT 	= 1<<2,	// Align text horizontally to right.
	};

	enum TextVerticalAlign {
		// Vertical align
		NVG_ALIGN_TOP 		= 1<<3,	// Align text vertically to top.
		NVG_ALIGN_MIDDLE	= 1<<4,	// Align text vertically to middle.
		NVG_ALIGN_BOTTOM	= 1<<5,	// Align text vertically to bottom.
		NVG_ALIGN_BASELINE	= 1<<6, // Default, align text vertically to baseline.
	};

	// returns font id that can be used later
	Font* addFont(const string& name, const string& filename);

	Font* getFont(const string& name);

	void drawText(const string& fontName, float x, float y,
				  const string& text, float fontSize);

	void drawText(Font* font, float x, float y,
				  const string& text, float fontSize);

	void drawTextBox(const string& fontName, float x, float y,
					 const string& text, float fontSize, float breakRowWidth);

	void drawTextBox(Font* font, float x, float y,
					 const string& text, float fontSize, float breakRowWidth);

	void setTextAlign(enum TextHorizontalAlign hor, enum TextVerticalAlign ver);

	ofRectangle getTextBounds(const string& fontName, float x, float y,
							  const string& text, float fontSize);

	ofRectangle getTextBounds(Font* font, float x, float y,
							  const string& text, float fontSize);

	ofRectangle getTextBoxBounds(const string& fontName, float x, float y,
								 const string& text, float fontSize, float breakRowWidth);

	ofRectangle getTextBoxBounds(Font* font, float x, float y,
								 const string& text, float fontSize, float breakRowWidth);

	void setFontBlur(float blur);



	// copy current OF matrix to nanovg
	void applyOFMatrix();

private:

	bool bInitialized;
	bool bInFrame;
	int frameWidth, frameHeight;
	float framePixRatio;

	NVGcontext* ctx;

	// fonts
	vector<ofxNanoVG::Font*> fonts;

	// apply OF color and stroke width
	void applyOFStyle();

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
