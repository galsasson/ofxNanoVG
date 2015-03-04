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

void ofxNanoVG::setup()
{
	if (bInitialized) {
		return;
	}

	ctx = nvgCreateGLES2(NVG_ANTIALIAS | NVG_DEBUG);
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

void ofxNanoVG::drawRoundedRect(float x, float y, float w, float h, float r)
{
	if (!bInFrame) {
		return;
	}


	setOFStyle();

//	nvgTr

	nvgBeginPath(ctx);
	nvgRoundedRect(ctx, x, y, w, h, r);

	doOFDraw();
}


void ofxNanoVG::setOFStyle()
{
	ofStyle style = ofGetStyle();
	ofFloatColor floatColor = style.color;
	NVGcolor color = nvgRGBAf(floatColor.r, floatColor.g, floatColor.b, floatColor.a);

	if (style.bFill) {
		nvgFillColor(ctx, color);
	}
	else {
		nvgStrokeColor(ctx, color);
		nvgStrokeWidth(ctx, style.lineWidth);
	}
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
