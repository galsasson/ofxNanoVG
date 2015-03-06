//
//  ofxNanoVG.cpp
//  sentopiary
//
//  Created by Gal Sasson on 12/27/14.
//
//

#include "ofxNanoVG.h"

#define NANOVG_GLES2_IMPLEMENTATION
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

	nvgBeginFrame(ctx, width, height, devicePixelRatio);
	bInFrame = true;
}

void ofxNanoVG::endFrame()
{
	if (!bInitialized) {
		return;
	}

	nvgEndFrame(ctx);
	bInFrame = false;
}

void ofxNanoVG::drawRect(const ofRectangle &rect)
{
	drawRect(rect.x, rect.y, rect.width, rect.height);
}

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

void ofxNanoVG::drawRoundedRect(const ofRectangle &rect, float r)
{
	return drawRoundedRect(rect.x, rect.y, rect.width, rect.height, r);
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

void ofxNanoVG::drawLine(float x1, float y1, float x2, float y2, enum ofxNanoVGlineParam cap, enum ofxNanoVGlineParam join)
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

void ofxNanoVG::drawLine(const ofPoint& p1, const ofPoint& p2, enum ofxNanoVGlineParam cap, enum ofxNanoVGlineParam join)
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

void ofxNanoVG::drawPolyline(const ofPolyline &line, enum ofxNanoVGlineParam cap, enum ofxNanoVGlineParam join)
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
