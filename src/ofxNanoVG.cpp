//
//  ofxNanoVG.cpp
//  sentopiary
//
//  Created by Gal Sasson on 12/27/14.
//
//

#include "ofxNanoVG.h"

#define NANOVG_GLES2_IMPLEMENTATION
#define FONTSTASH_IMPLEMENTATION
#include "nanovg.h"
#include "nanovg_gl.h"

ofxNanoVG::~ofxNanoVG()
{
	ofLogWarning("ofxNanoVG","In destructor");

	if (!bInitialized) {
		return;
	}

	nvgDeleteGLES2(ctx);
}

void ofxNanoVG::setup(bool debug)
{
	if (bInitialized) {
		return;
	}

	ctx = nvgCreateGLES2(NVG_ANTIALIAS | (debug?NVG_DEBUG:0));

	if (!ctx) {
		ofLogError("error creating nanovg context");
		return;
	}

	bInitialized = true;
}

void ofxNanoVG::beginFrame(int width, int height, float devicePixelRatio)
{
	if (!bInitialized) {
		return;
	}

	frameWidth = width;
	frameHeight = height;
	framePixRatio = devicePixelRatio;

	nvgBeginFrame(ctx, width, height, devicePixelRatio);
	bInFrame = true;
}

void ofxNanoVG::endFrame()
{
	if (!bInitialized) {
		return;
	}

	nvgEndFrame(ctx);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	bInFrame = false;
}

void ofxNanoVG::flush()
{
	if (!bInFrame) {
		return;
	}

	endFrame();
	beginFrame(frameWidth, frameHeight, framePixRatio);
}

/*******************************************************************************
 * Shapes
 ******************************************************************************/

void ofxNanoVG::drawRect(float x, float y, float w, float h)
{
	if (!bInFrame) {
		return;
	}

	applyOFStyle();
	applyOFMatrix();

	nvgBeginPath(ctx);
	nvgRect(ctx, x, y, w, h);

	doOFDraw();
}

void ofxNanoVG::drawRoundedRect(float x, float y, float w, float h, float r)
{
	if (!bInFrame) {
		return;
	}

	applyOFStyle();
	applyOFMatrix();

	nvgBeginPath(ctx);
	nvgRoundedRect(ctx, x, y, w, h, r);

	doOFDraw();
}

void ofxNanoVG::drawEllipse(float cx, float cy, float rx, float ry)
{
	if (!bInFrame) {
		return;
	}

	applyOFStyle();
	applyOFMatrix();

	nvgBeginPath(ctx);
	nvgEllipse(ctx, cx, cy, rx, ry);

	doOFDraw();
}

void ofxNanoVG::drawCircle(float cx, float cy, float r)
{
	if (!bInFrame) {
		return;
	}

	applyOFStyle();
	applyOFMatrix();

	nvgBeginPath(ctx);
	nvgCircle(ctx, cx, cy, r);

	doOFDraw();
}

void ofxNanoVG::drawLine(float x1, float y1, float x2, float y2, enum LineParam cap, enum LineParam join)
{
	if (!bInFrame) {
		return;
	}

	applyOFStyle();
	applyOFMatrix();

	nvgLineCap(ctx, cap);
	nvgLineJoin(ctx, join);

	nvgBeginPath(ctx);
	nvgMoveTo(ctx, x1, y1);
	nvgLineTo(ctx, x2, y2);

	nvgStroke(ctx);
}

void ofxNanoVG::drawLine(const ofPoint& p1, const ofPoint& p2, enum LineParam cap, enum LineParam join)
{
	if (!bInFrame) {
		return;
	}

	applyOFStyle();
	applyOFMatrix();

	nvgLineCap(ctx, cap);
	nvgLineJoin(ctx, join);

	nvgBeginPath(ctx);
	nvgMoveTo(ctx, p1.x, p1.y);
	nvgLineTo(ctx, p2.x, p2.y);

	nvgStroke(ctx);
}

void ofxNanoVG::drawPolyline(const ofPolyline &line, enum LineParam cap, enum LineParam join)
{
	if (!bInFrame) {
		return;
	}

	applyOFStyle();
	applyOFMatrix();

	nvgLineCap(ctx, cap);
	nvgLineJoin(ctx, join);

	const vector<ofPoint>& verts = line.getVertices();
	nvgBeginPath(ctx);
	nvgMoveTo(ctx, verts[0].x, verts[1].y);
	for (ofPoint p : verts) {
		nvgLineTo(ctx, p.x, p.y);
	}

	nvgStroke(ctx);
}


/*******************************************************************************
 * Text
 ******************************************************************************/

ofxNanoVG::Font* ofxNanoVG::addFont(const string &name, const string &filename)
{
	int fontId = nvgCreateFont(ctx, name.c_str(), ofToDataPath(filename).c_str());
	if (fontId < 0) {
		ofLogError("ofxNanoVG::addFont", "could not create font %s from file: %s", name.c_str(), filename.c_str());
		return NULL;
	}

	Font* font = new Font();
	font->id = fontId;
	font->name = name;
	font->letterSpacing = 0;
	font->lineHeight = 1.0f;
	fonts.push_back(font);

	return font;
}

ofxNanoVG::Font* ofxNanoVG::getFont(const string &name)
{
	for (int i=0; i<fonts.size(); i++) {
		if (fonts[i]->name == name) {
			return fonts[i];
		}
	}

	return NULL;
}

void ofxNanoVG::drawText(const string &fontName, float x, float y, const string &text, float fontSize)
{
	Font* font = getFont(fontName);
	if (font == NULL) {
		ofLogError("ofxNanoVG::drawText", "cannot find font: %s", fontName.c_str());
		return;
	}

	drawText(font, x, y, text, fontSize);
}

void ofxNanoVG::drawText(ofxNanoVG::Font *font, float x, float y, const string &text, float fontSize)
{
	if (font == NULL) {
		ofLogError("ofxNanoVG::drawText", "font == NULL");
		return;
	}

	nvgFontFaceId(ctx, font->id);
	nvgTextLetterSpacing(ctx, font->letterSpacing);
	nvgFontSize(ctx, fontSize);

	applyOFStyle();
	applyOFMatrix();

	nvgText(ctx, x, y, text.c_str(), NULL);
}

void ofxNanoVG::drawTextBox(const string &fontName, float x, float y, const string &text, float fontSize, float breakRowWidth)
{
	Font* font = getFont(fontName);
	if (font == NULL) {
		ofLogError("ofxNanoVG::drawTextBox", "cannot find font: %s", fontName.c_str());
		return;
	}

	drawTextBox(font, x, y, text, fontSize, breakRowWidth);
}

void ofxNanoVG::drawTextBox(ofxNanoVG::Font *font, float x, float y, const string &text, float fontSize, float breakRowWidth)
{
	if (font == NULL) {
		ofLogError("ofxNanoVG::drawTextBox", "font == NULL");
		return;
	}

	nvgFontFaceId(ctx, font->id);
	nvgTextLetterSpacing(ctx, font->letterSpacing);
	nvgTextLineHeight(ctx, font->lineHeight);
	nvgFontSize(ctx, fontSize);

	applyOFStyle();
	applyOFMatrix();

	nvgTextBox(ctx, x, y, breakRowWidth, text.c_str(), NULL);
}

void ofxNanoVG::setTextAlign(enum TextHorizontalAlign hor, enum TextVerticalAlign ver)
{
	nvgTextAlign(ctx, hor | ver);
}

ofRectangle ofxNanoVG::getTextBounds(const string &fontName, float x, float y, const string &text, float fontSize)
{
	Font* font = getFont(fontName);
	if (font == NULL) {
		ofLogError("ofxNanoVG::getTextBounds", "cannot find font: %s", fontName.c_str());
		return ofRectangle();
	}

	return getTextBounds(font, x, y, text, fontSize);
}

ofRectangle ofxNanoVG::getTextBounds(ofxNanoVG::Font *font, float x, float y, const string &text, float fontSize)
{
	if (font == NULL) {
		ofLogError("ofxNanoVG::getTextBounds", "font == NULL");
		return ofRectangle();
	}

	nvgFontFaceId(ctx, font->id);
	nvgTextLetterSpacing(ctx, font->letterSpacing);
	nvgFontSize(ctx, fontSize);

	float bounds[4];
	nvgTextBounds(ctx, x, y, text.c_str(), NULL, bounds);

	return ofRectangle(bounds[0], bounds[1], bounds[2]-bounds[0], bounds[3]-bounds[1]);
}

ofRectangle ofxNanoVG::getTextBoxBounds(const string &fontName, float x, float y, const string &text, float fontSize, float breakRowWidth)
{
	Font* font = getFont(fontName);
	if (font == NULL) {
		ofLogError("ofxNanoVG::getTextBoxBounds", "cannot find font: %s", fontName.c_str());
		return ofRectangle();
	}

	return getTextBoxBounds(font, x, y, text, fontSize, breakRowWidth);
}

ofRectangle ofxNanoVG::getTextBoxBounds(ofxNanoVG::Font *font, float x, float y, const string &text, float fontSize, float breakRowWidth)
{
	if (font == NULL) {
		ofLogError("ofxNanoVG::getTextBoxBounds", "font == NULL");
		return ofRectangle();
	}

	nvgFontFaceId(ctx, font->id);
	nvgTextLetterSpacing(ctx, font->letterSpacing);
	nvgTextLineHeight(ctx, font->lineHeight);
	nvgFontSize(ctx, fontSize);

	float bounds[4];
	nvgTextBoxBounds(ctx, x, y, breakRowWidth, text.c_str(), NULL, bounds);

	return ofRectangle(bounds[0], bounds[1], bounds[2]-bounds[0], bounds[3]-bounds[1]);
}


void ofxNanoVG::setFontBlur(float blur)
{
	nvgFontBlur(ctx, blur);
}


//------------------------------------------------------------------
// private
//------------------------------------------------------------------


void ofxNanoVG::applyOFStyle()
{
	ofStyle style = ofGetStyle();
	ofFloatColor floatColor = style.color;
	NVGcolor color = nvgRGBAf(floatColor.r, floatColor.g, floatColor.b, floatColor.a);

	nvgFillColor(ctx, color);
	nvgStrokeColor(ctx, color);
	nvgStrokeWidth(ctx, style.lineWidth);
}

/******
 * applyOFMatrix
 *
 * Take OF model-view matrix and apply the affine transformations
 * on nanovg matrix
 */
void ofxNanoVG::applyOFMatrix()
{
	ofMatrix4x4 ofMatrix = ofGetCurrentMatrix(OF_MATRIX_MODELVIEW);
	ofVec2f viewSize = ofVec2f(ofGetViewportWidth(), ofGetViewportHeight());

	ofVec2f translate = ofVec2f(ofMatrix(3, 0), ofMatrix(3, 1)) + viewSize/2;
	ofVec2f scale(ofMatrix(0, 0), ofMatrix(1, 1));
	ofVec2f skew(ofMatrix(0, 1), ofMatrix(1, 0));

	// handle OF style vFlipped inside FBO
	if (ofGetCurrentRenderer()->getCurrentOrientationMatrix()(1, 1) == 1) {
		translate.y = ofGetViewportHeight() - translate.y;
		scale.y *= -1;
	}

	nvgResetTransform(ctx);
	nvgTransform(ctx, scale.x, -skew.y, -skew.x,
				 scale.y, translate.x, translate.y);
}

void ofxNanoVG::doOFDraw()
{
	ofStyle style = ofGetStyle();
	if (style.bFill) {
		nvgFill(ctx);
	}
	else {
		nvgStroke(ctx);
	}
}
