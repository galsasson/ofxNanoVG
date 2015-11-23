//
//  ofxNanoVG.cpp
//  sentopiary
//
//  Created by Gal Sasson on 12/27/14.
//
//

#include "ofxNanoVG.h"

//#define ALWAYS_APPLY_OF_MATRIX

#define NVG_DISABLE_FACE_CULL_FOR_TRIANGLES

#define FONTSTASH_IMPLEMENTATION
#include "nanovg.h"
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

#if !defined(NANOVG_GL3_IMPLEMENTATION) && !defined(NANOVG_GLES2_IMPLEMENTATION)
#error "ofxNanoVG: Please add one of the following definitions to your project NANOVG_GL3_IMPLEMENTATION, NANOVG_GLES2_IMPLEMENTATION"
#endif

#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"

ofxNanoVG::~ofxNanoVG()
{
	if (!bInitialized) {
		return;
	}

#ifdef NANOVG_GL3_IMPLEMENTATION
	nvgDeleteGL3(ctx);
#elif defined NANOVG_GLES2_IMPLEMENTATION
	nvgDeleteGLES2(ctx);
#endif
}

void ofxNanoVG::setup(bool stencilStrokes, bool debug)
{
	if (bInitialized) {
		return;
	}

#ifdef NANOVG_GL3_IMPLEMENTATION
	ctx = nvgCreateGL3(NVG_ANTIALIAS | (stencilStrokes?NVG_STENCIL_STROKES:0) | (debug?NVG_DEBUG:0));
#elif defined NANOVG_GLES2_IMPLEMENTATION
	ctx = nvgCreateGLES2(NVG_ANTIALIAS | (stencilStrokes?NVG_STENCIL_STROKES:0) | (debug?NVG_DEBUG:0));
#endif

	if (!ctx) {
		ofLogError("error creating nanovg context");
		return;
	}

	// set defaults
	nvgLineCap(ctx, NVG_BUTT);
	nvgLineJoin(ctx, NVG_MITER);

	bInitialized = true;
}

void ofxNanoVG::beginFrame(int width, int height, float devicePixelRatio)
{
	if (!bInitialized) {
		return;
	}
	
	if (bInFrame) {
		ofLogError("ofxNanoVG") << "beginFrame was called while in a frame";
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
	
#ifdef ADD_OF_PATCH_FOR_NANOVG
	ofGetCurrentRenderer()->setCurrentShaderDirty();
#endif
}

void ofxNanoVG::pushFrame()
{
	if (!bInFrame) {
		ofLogError("ofxNanoVG") << "pushFrame was called outside of a frame";
		return;
	}
	
	Settings set;
	set.width = frameWidth;
	set.height = frameHeight;
	set.devicePixelRatio = framePixRatio;
	framesStack.push(set);

	endFrame();
}

void ofxNanoVG::popFrame()
{
	if (framesStack.empty()) {
		ofLogNotice("ofxNanoVG") << "trying to pop empty frame stack";
		return;
	}
	
	Settings set = framesStack.top();
	framesStack.pop();

	beginFrame(set.width, set.height, set.devicePixelRatio);
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
#ifdef ALWAYS_APPLY_OF_MATRIX
	applyOFMatrix();
#endif

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
#ifdef ALWAYS_APPLY_OF_MATRIX
	applyOFMatrix();
#endif

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
#ifdef ALWAYS_APPLY_OF_MATRIX
	applyOFMatrix();
#endif

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
#ifdef ALWAYS_APPLY_OF_MATRIX
	applyOFMatrix();
#endif

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
#ifdef ALWAYS_APPLY_OF_MATRIX
	applyOFMatrix();
#endif

	nvgLineCap(ctx, cap);
	nvgLineJoin(ctx, join);

	nvgBeginPath(ctx);
	nvgMoveTo(ctx, x1, y1);
	nvgLineTo(ctx, x2, y2);

	nvgStroke(ctx);
}

void ofxNanoVG::drawPolyline(const ofPolyline &line, enum LineParam cap, enum LineParam join)
{
	if (!bInFrame) {
		return;
	}

	if (line.size() == 0) {
		return;
	}

	applyOFStyle();
#ifdef ALWAYS_APPLY_OF_MATRIX
	applyOFMatrix();
#endif

	nvgLineCap(ctx, cap);
	nvgLineJoin(ctx, join);

	const vector<ofPoint>& verts = line.getVertices();
	nvgBeginPath(ctx);
	nvgMoveTo(ctx, verts[0].x, verts[0].y);
	for (int i=1; i<verts.size(); i++) {
		nvgLineTo(ctx, verts[i].x, verts[i].y);
	}

	nvgStroke(ctx);
}

void ofxNanoVG::fillPolyline(const ofPolyline &line, enum LineParam cap, enum LineParam join)
{
	if (!bInFrame) {
		return;
	}

	if (line.size() == 0) {
		return;
	}

	applyOFStyle();
#ifdef ALWAYS_APPLY_OF_MATRIX
	applyOFMatrix();
#endif

	nvgLineCap(ctx, cap);
	nvgLineJoin(ctx, join);

	const vector<ofPoint>& verts = line.getVertices();
	nvgBeginPath(ctx);
	nvgMoveTo(ctx, verts[0].x, verts[0].y);
	for (int i=1; i<verts.size(); i++) {
		nvgLineTo(ctx, verts[i].x, verts[i].y);
	}

	nvgFill(ctx);
}

void ofxNanoVG::drawPath(const ofPath& path, float x, float y, enum LineParam cap, enum LineParam join)
{
	if (!bInFrame) {
		return;
	}

	if (x!=0 || y!=0) {
		nvgTranslate(ctx, x, y);
	}

	applyOFStyle();
#ifdef ALWAYS_APPLY_OF_MATRIX
	applyOFMatrix();
#endif

	nvgLineCap(ctx, cap);
	nvgLineJoin(ctx, join);
	nvgBeginPath(ctx);
	for (const ofPath::Command& c : path.getCommands()) {
		switch (c.type) {
			case ofPath::Command::moveTo:
				nvgMoveTo(ctx, c.to.x, c.to.y);
				break;
			case ofPath::Command::lineTo:
				nvgLineTo(ctx, c.to.x, c.to.y);
				break;
			case ofPath::Command::bezierTo:
				nvgBezierTo(ctx, c.cp1.x, c.cp1.y, c.cp2.x, c.cp2.y, c.to.x, c.to.y);
				break;
			default:
				break;
		}

	}
	nvgStroke(ctx);

	if (x!=0 || y!=0) {
		nvgTranslate(ctx, -x, -y);
	}
}

void ofxNanoVG::beginShape()
{
	if (!bInFrame) {
		return;
	}

	if (bInShape) {
		ofLogError("ofxNanoVG") << "call to beginShape while already inside shape drawing"<<endl;
		return;
	}

	bInShape = true;
	vertexCount = 0;

	nvgBeginPath(ctx);
}

void ofxNanoVG::endShape()
{
	if (!bInShape) {
		ofLogError("ofxNanoVG") << "call to endShape without previous call to beginShape"<<endl;
		return;
	}

	doOFDraw();
	bInShape = false;
}

void ofxNanoVG::endShapeStroke()
{
	if (!bInShape) {
		ofLogError("ofxNanoVG") << "call to endShape without previous call to beginShape"<<endl;
		return;
	}
	
	nvgStroke(ctx);
	bInShape = false;
}

void ofxNanoVG::endShapeFill()
{
	if (!bInShape) {
		ofLogError("ofxNanoVG") << "call to endShape without previous call to beginShape"<<endl;
		return;
	}
	
	nvgFill(ctx);
	bInShape = false;
}

void ofxNanoVG::moveTo(float x, float y)
{
	if (!bInShape) {
		ofLogError("ofxNanoVG") << "call to moveTo without previous call to beginShape"<<endl;
		return;
	}

	nvgMoveTo(ctx, x, y);
	vertexCount++;
}

void ofxNanoVG::lineTo(float x, float y)
{
	if (!bInShape) {
		ofLogError("ofxNanoVG") << "call to lineTo without previous call to beginShape"<<endl;
		return;
	}

	nvgLineTo(ctx, x, y);
	vertexCount++;
}

void ofxNanoVG::bezierTo(float cx1, float cy1, float cx2, float cy2, float x, float y)
{
	if (!bInShape) {
		ofLogError("ofxNanoVG") << "call to lineTo without previous call to beginShape"<<endl;
		return;
	}

	nvgBezierTo(ctx, cx1, cy1, cx2, cy2, x, y);
}

void ofxNanoVG::vertex(float x, float y)
{
	if (!bInShape) {
		ofLogError("ofxNanoVG") << "call to vertex(x,y) without beginShape"<<endl;
		return;
	}

	if (vertexCount==0) {
		nvgMoveTo(ctx, x, y);
	}
	else {
		nvgLineTo(ctx, x, y);
	}

	vertexCount++;
}

void ofxNanoVG::bezierVertex(float cx1, float cy1, float cx2, float cy2, float x, float y)
{
	if (!bInShape) {
		ofLogError("ofxNanoVG") << "call to bezierVertex(x,y) without beginShape"<<endl;
		return;
	}

	if (vertexCount==0) {
		nvgMoveTo(ctx, 0, 0);
	}

	nvgBezierTo(ctx, cx1, cy1, cx2, cy2, x, y);

	vertexCount++;
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
#ifdef ALWAYS_APPLY_OF_MATRIX
	applyOFMatrix();
#endif

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
#ifdef ALWAYS_APPLY_OF_MATRIX
	applyOFMatrix();
#endif

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


/*******************************************************************************
 * SVG
 ******************************************************************************/

NSVGimage* ofxNanoVG::parseSvgFile(const string& filename, const string& units, float dpi)
{
	return nsvgParseFromFile(ofToDataPath(filename).c_str(), units.c_str(), dpi);
}

void ofxNanoVG::drawSvg(NSVGimage* svg, float x, float y)
{
	if (svg == NULL) {
		return;
	}

	if (x!=0 || y!=0) {
		nvgTranslate(ctx, x, y);
	}

	NSVGshape* shape = svg->shapes;
	while (shape != NULL) {
		NSVGpath* path = shape->paths;
		while (path != NULL) {
			beginShape();
			for (int i=0; i<path->npts; i++) {
				vertex(path->pts[i*2], path->pts[i*2+1]);
			}
			endShape();
			path = path->next;		// next path
		}
		shape = shape->next; 		// next shape
	}

	if (x!=0 || y!=0) {
		nvgTranslate(ctx, -x, -y);
	}
}

void ofxNanoVG::freeSvg(NSVGimage* svg)
{
	nsvgDelete(svg);
}

/******
 * Paint
 */

void ofxNanoVG::setFillColor(const ofFloatColor &c)
{
	if (!bInitialized) {
		return;
	}

	nvgFillColor(ctx, toNVGcolor(c));
}

void ofxNanoVG::setFillPaint(const NVGpaint &paint)
{
	if (!bInitialized) {
		return;
	}

	nvgFillPaint(ctx, paint);
}

void ofxNanoVG::setStrokeColor(const ofFloatColor &c)
{
	if (!bInitialized) {
		return;
	}

	nvgStrokeColor(ctx, toNVGcolor(c));
}

void ofxNanoVG::setStrokePaint(const NVGpaint &paint)
{
	if (!bInitialized) {
		return;
	}

	nvgStrokePaint(ctx, paint);
}

NVGpaint ofxNanoVG::getLinearGradient(float sx, float sy, float ex, float ey, const ofColor &c1, const ofColor &c2)
{
	return nvgLinearGradient(ctx, sx, sy, ex, ey, toNVGcolor(c1), toNVGcolor(c2));
}

NVGcolor ofxNanoVG::toNVGcolor(const ofFloatColor& c)
{
	return nvgRGBAf(c.r, c.g, c.b, c.a);
}

/******
 * applyOFMatrix
 *
 * Take OF model-view matrix and apply the affine transformations
 * on nanovg matrix
 */
void ofxNanoVG::applyOFMatrix()
{
	if (!bInitialized) {
		return;
	}

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

void ofxNanoVG::resetMatrix()
{
	if (!bInitialized) {
		return;
	}

	nvgResetTransform(ctx);
}

//------------------------------------------------------------------
// private
//------------------------------------------------------------------


void ofxNanoVG::applyOFStyle()
{
	ofStyle style = ofGetStyle();
	NVGcolor color = toNVGcolor(style.color);

	nvgFillColor(ctx, color);
	nvgStrokeColor(ctx, color);
	nvgStrokeWidth(ctx, style.lineWidth);
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
