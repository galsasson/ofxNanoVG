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
#include "nanovg.h"

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

	// must call beginPath before drawing
	void beginPath();
	
	// call fillPath or strokePath after drawing with the functions below to fill/stroke the path
	void fillPath();
	void strokePath();

	void rect(const ofRectangle& r);
	void rect(float x, float y, float w, float h);
	void roundedRect(const ofRectangle& r, float ang);
	void roundedRect(float x, float y, float w, float h, float r);
	void ellipse(const ofVec2f& p, float rx, float ry);
	void ellipse(float cx, float cy, float rx, float ry);
	void circle(const ofVec2f& p, float r);
	void circle(float cx, float cy, float r);
	void line(const ofVec2f& p1, const ofVec2f& p2);
	void line(float x1, float y1, float x2, float y2);
	void moveTo(const ofVec2f& p);
	void moveTo(float x, float y);
	void lineTo(const ofVec2f& p);
	void lineTo(float x, float y);
	void bezierTo(float cx1, float cy1, float cx2, float cy2, float x, float y);
	void followPolyline(const ofPolyline& line);
	void followPath(const ofPath& path, float x=0, float y=0);

	/******
	 * For convenience
	 */

	void strokeRect(float x, float y, float w, float h, const ofColor& c);
	void fillRect(float x, float y, float w, float h, const ofColor& c);
	void strokeRoundedRect(float x, float y, float w, float h, float r, const ofColor& c);
	void fillRoundedRect(float x, float y, float w, float h, float r, const ofColor& c);
	
	/******
	 * Style
	 */
	void setStrokeWidth(float width);
	void setLineCap(enum LineParam cap);
	void setLineJoin(enum LineParam join);
	void setFillColor(const ofFloatColor& c);
	void setFillPaint(const NVGpaint& paint);
	void setStrokeColor(const ofFloatColor& c);
	void setStrokePaint(const NVGpaint& paint);
	NVGpaint getLinearGradientPaint(float sx, float sy, float ex, float ey, const ofColor& c1, const ofColor& c2);
	NVGpaint getTexturePaint(const ofTexture& tex);
	static inline NVGcolor toNVGcolor(const ofFloatColor& c);

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
	void drawText(const string& fontName, float x, float y, const string& text, float fontSize);
	void drawText(Font* font, float x, float y, const string& text, float fontSize);
	void drawTextBox(const string& fontName, float x, float y, const string& text, float fontSize, float breakRowWidth);
	void drawTextBox(Font* font, float x, float y, const string& text, float fontSize, float breakRowWidth);
	void setTextAlign(enum TextHorizontalAlign hor, enum TextVerticalAlign ver);
	ofRectangle getTextBounds(const string& fontName, float x, float y, const string& text, float fontSize);
	ofRectangle getTextBounds(Font* font, float x, float y, const string& text, float fontSize);
	ofRectangle getTextBoxBounds(const string& fontName, float x, float y, const string& text, float fontSize, float breakRowWidth);
	ofRectangle getTextBoxBounds(Font* font, float x, float y, const string& text, float fontSize, float breakRowWidth);
	void setFontBlur(float blur);
	
	/******
	 * SVG
	 */
	
	NSVGimage* parseSvgFile(const string& filename, const string& units, float dpi);
	void followSvg(NSVGimage* svg, float x=0, float y=0);
	void freeSvg(NSVGimage* svg);
	
	// copy current OF matrix to nanovg
	void applyOFMatrix();
	void resetMatrix();
	// apply OF color and stroke width
	void applyOFStyle();

private:

	bool bInitialized;
	bool bInFrame;
	int frameWidth, frameHeight;
	float framePixRatio;

	// for shapes
	int vertexCount;


	NVGcontext* ctx;

	// fonts
	vector<ofxNanoVG::Font*> fonts;

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
