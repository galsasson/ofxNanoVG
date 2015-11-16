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
#include "nanosvg.h"

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

	void setup(bool stencilStrokes=false, bool debug=false);

	struct Settings {
		int width;
		int height;
		float devicePixelRatio;
	};

	void beginFrame(int width, int height, float devicePixelRatio);
	void endFrame();
	void flush();
	void pushFrame();
	void popFrame();
	std::stack<Settings> framesStack;

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
	void drawLine(const ofPoint& p1, const ofPoint& p2,
				  enum LineParam cap=NVG_BUTT, enum LineParam join=NVG_MITER) {
		drawLine(p1.x, p1.y, p2.x, p2.y, cap, join);
	}
	void drawLine(float x1, float y1, float x2, float y2,
				  enum LineParam cap=NVG_BUTT, enum LineParam join=NVG_MITER);
	void drawPolyline(const ofPolyline& line,
					  enum LineParam cap=NVG_BUTT, enum LineParam join=NVG_MITER);
	void fillPolyline(const ofPolyline& line,
					  enum LineParam cap=NVG_BUTT, enum LineParam join=NVG_MITER);
	void drawPath(const ofPath& path, enum LineParam cap=NVG_BUTT, enum LineParam join=NVG_MITER);
	void beginShape();
	void endShape();
	void endShapeStroke();
	void endShapeFill();
	void moveTo(const ofVec2f& p) {
		moveTo(p.x, p.y);
	}
	void moveTo(float x, float y);
	void lineTo(const ofVec2f& p) {
		lineTo(p.x, p.y);
	}
	void lineTo(float x, float y);
	void vertex(const ofVec2f& p) {
		vertex(p.x, p.y);
	}
	void vertex(float x, float y);
	void bezierVertex(const ofVec2f& cp1, const ofVec2f& cp2, const ofVec2f& p) {
		bezierVertex(cp1.x, cp1.y, cp2.x, cp2.y, p.x, p.y);
	}
	void bezierVertex(float cx1, float cy1, float cx2, float cy2, float x, float y);
	

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

	
	/******
	 * SVG
	 */
	
	NSVGimage* parseSvgFile(const string& filename, const string& units, float dpi);
	void drawSvg(NSVGimage* svg, float x=0, float y=0);
	void freeSvg(NSVGimage* svg);
	
	// copy current OF matrix to nanovg
	void applyOFMatrix();

private:

	bool bInitialized;
	bool bInFrame;
	int frameWidth, frameHeight;
	float framePixRatio;

	// for shapes
	bool bInShape;
	int vertexCount;


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
		bInShape(false),
		ctx(NULL) {}

	// make sure there are no copies
	ofxNanoVG(ofxNanoVG const&);
	void operator=(ofxNanoVG const&);


};

#endif /* defined(__sentopiary__ofxNanoVG__) */
