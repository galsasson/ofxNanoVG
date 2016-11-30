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
	inline void beginPath() {
		nvgBeginPath(ctx);
	}
	
	// call fillPath or strokePath after drawing with the functions below to fill/stroke the path
	inline void strokePath() {
		nvgStroke(ctx);
	}
	inline void strokePath(const ofColor& c) {
		setStrokeColor(c);
		strokePath();
	}
	
	inline void fillPath() {
		nvgFill(ctx);
	}
	inline void fillPath(const ofColor& c) {
		setFillColor(c);
		fillPath();
	}
	
	inline void rect(const ofRectangle& r) { rect(r.x, r.y, r.width, r.height); }
	inline void rect(float x, float y, float w, float h) {
		nvgRect(ctx, x, y, w, h);
	}
	
	inline void roundedRect(const ofRectangle &r, float ang) { roundedRect(r.x, r.y, r.width, r.height, ang); }
	inline void roundedRect(float x, float y, float w, float h, float r) {
		nvgRoundedRect(ctx, x, y, w, h, r);
	}
	inline void roundedRect(const ofRectangle &r, float ang_tl, float ang_tr, float ang_br, float ang_bl) { roundedRect(r.x, r.y, r.width, r.height, ang_tl, ang_tr, ang_br, ang_bl); }
	inline void roundedRect(float x, float y, float w, float h, float r_tl, float r_tr, float r_br, float r_bl) {
		nvgRoundedRect4(ctx, x, y, w, h, r_tl, r_tr, r_br, r_bl);
	}

	inline void ellipse(const ofVec2f& p, float rx, float ry) { ellipse(p.x, p.y, rx, ry); }
	inline void ellipse(float cx, float cy, float rx, float ry) {
		nvgEllipse(ctx, cx, cy, rx, ry);
	}
	
	inline void circle(const ofVec2f& p, float r) { circle(p.x, p.y, r); }
	inline void circle(float cx, float cy, float r) {
		nvgCircle(ctx, cx, cy, r);
	}
	
	inline void arc(const ofVec2f& p, float r, float a0, float a1, int dir) { arc(p.x, p.y, r, a0, a1, dir); }
	inline void arc(float cx, float cy, float r, float a0, float a1, int dir) {
		nvgArc(ctx, cx, cy, r, ofDegToRad(a0-90), ofDegToRad(a1-90), dir);
	}

	inline void line(const ofVec2f& p1, const ofVec2f& p2) { line(p1.x, p1.y, p2.x, p2.y); }
	inline void line(float x1, float y1, float x2, float y2) {
		nvgMoveTo(ctx, x1, y1);
		nvgLineTo(ctx, x2, y2);
	}
	
	inline void moveTo(const ofVec2f& p) { moveTo(p.x, p.y); }
	inline void moveTo(float x, float y) {
		nvgMoveTo(ctx, x, y);
	}
	
	inline void lineTo(const ofVec2f& p) { lineTo(p.x, p.y); }
	inline void lineTo(float x, float y) {
		nvgLineTo(ctx, x, y);
	}
	
	inline void bezierTo(const ofVec2f& cp1, const ofVec2f& cp2, const ofVec2f& dst) { bezierTo(cp1.x, cp1.y, cp2.x, cp2.y, dst.x, dst.y); }
	inline void bezierTo(float cx1, float cy1, float cx2, float cy2, float x, float y) {
		nvgBezierTo(ctx, cx1, cy1, cx2, cy2, x, y);
	}
	
	void followPolyline(const ofPolyline& line);
	void followPolylineDashed(const ofPolyline& line, float onpx=4, float offpx=4);
	void followPath(const ofPath& path, float x=0, float y=0);

	/******
	 * For convenience
	 */

	void strokeLine(float x1, float y1, float x2, float y2, const ofColor& c, float width=1);
	inline void strokeLine(const ofVec2f& p1, const ofVec2f& p2, const ofColor& c, float width=1) {
		strokeLine(p1.x, p1.y, p2.x, p2.y, c, width);
	}
	void strokeRect(float x, float y, float w, float h, const ofColor& c, float width=1);
	void fillRect(float x, float y, float w, float h, const ofColor& c);
	void fillRect(const ofRectangle& rect, const ofColor& c) {
		fillRect(rect.x, rect.y, rect.width, rect.height, c);
	}
	void strokeRoundedRect(float x, float y, float w, float h, float r, const ofColor& c, float width=1);
	void fillRoundedRect(float x, float y, float w, float h, float r, const ofColor& c);
	void strokeRoundedRect(float x, float y, float w, float h, float r_tl, float r_tr, float r_br, float r_bl, const ofColor& c, float width=1);
	void fillRoundedRect(float x, float y, float w, float h, float r_tl, float r_tr, float r_br, float r_bl, const ofColor& c);
	void strokeEllipse(float cx, float cy, float rx, float ry, const ofColor& c, float width=1);
	void fillEllipse(float cx, float cy, float rx, float ry, const ofColor& c);
	void strokeCircle(float cx, float cy, float r, const ofColor& c, float width=1);
	inline void strokeCircle(const ofVec2f& p, float r, const ofColor& c, float width=1) { strokeCircle(p.x, p.y, r, c, width); }
	void fillCircle(float cx, float cy, float r, const ofColor& c);
	inline void fillCircle(const ofVec2f& p, float r, const ofColor& c) { fillCircle(p.x, p.y, r, c); }
	void strokeArc(float cx, float cy, float r, float a0, float a1, int dir, const ofColor& c, float width=1);
	void fillArc(float cx, float cy, float r, float a0, float a1, int dir, const ofColor& c);
	void strokePolyline(const ofPolyline& line, const ofColor& c, float width=1);
	void fillPolyline(const ofPolyline& line, const ofColor& c);
	
	/******
	 * Style
	 */
	inline void setStrokeWidth(float width) {
		nvgStrokeWidth(ctx, width);
	}
	
	inline void setLineCap(enum LineParam cap) {
		nvgLineCap(ctx, cap);
	}
	
	inline void setLineJoin(enum LineParam join) {
		nvgLineJoin(ctx, join);
	}
	
	inline void setFillColor(const ofFloatColor &c) {
		nvgFillColor(ctx, toNVGcolor(c));
	}
	
	inline void setFillPaint(const NVGpaint &paint) {
		nvgFillPaint(ctx, paint);
	}
	
	inline void setStrokeColor(const ofFloatColor &c) {
		nvgStrokeColor(ctx, toNVGcolor(c));
	}
	
	inline void setStrokePaint(const NVGpaint &paint) {
		nvgStrokePaint(ctx, paint);
	}
	
	inline NVGpaint getLinearGradientPaint(float sx, float sy, float ex, float ey, const ofColor &c1, const ofColor &c2) {
		return nvgLinearGradient(ctx, sx, sy, ex, ey, toNVGcolor(c1), toNVGcolor(c2));
	}
	
	NVGpaint getTexturePaint(const ofTexture& tex);
	static inline NVGcolor toNVGcolor(const ofFloatColor& c) {
		return nvgRGBAf(c.r, c.g, c.b, c.a);
	}

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
	float drawText(const string& fontName, float x, float y, const string& text, float fontSize);
	float drawText(Font* font, float x, float y, const string& text, float fontSize);
	void drawTextBox(const string& fontName, float x, float y, const string& text, float fontSize, float breakRowWidth, float lineHeight=-1);
	void drawTextBox(Font* font, float x, float y, const string& text, float fontSize, float breakRowWidth, float lineHeight=-1);
	float drawTextOnArc(const string& fontName, float cx, float cy, float radius, float startAng, int dir, float spacing, const string& text, float fontSize, bool justMeasure=false);	// returns the radial travel in degrees
	void setTextAlign(enum TextHorizontalAlign hor, enum TextVerticalAlign ver);
	ofRectangle getTextBounds(const string& fontName, float x, float y, const string& text, float fontSize);
	ofRectangle getTextBounds(Font* font, float x, float y, const string& text, float fontSize);
	ofRectangle getTextBoxBounds(const string& fontName, float x, float y, const string& text, float fontSize, float breakRowWidth, float lineHeight=-1);
	ofRectangle getTextBoxBounds(Font* font, float x, float y, const string& text, float fontSize, float breakRowWidth, float lineHeight=-1);
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
	void translateMatrix(float x, float y);
	void enableScissor(float x, float y, float w, float h);
	void disableScissor();

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
