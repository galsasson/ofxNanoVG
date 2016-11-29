//
//  ofxNanoVG.cpp
//  sentopiary
//
//  Created by Gal Sasson on 12/27/14.
//
//

#include "ofxNanoVG.h"

#define NVG_DISABLE_FACE_CULL_FOR_TRIANGLES

#define FONTSTASH_IMPLEMENTATION
#include "nanovg.h"
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

#if !defined(NANOVG_GL3_IMPLEMENTATION) && !defined(NANOVG_GLES2_IMPLEMENTATION) && !defined(NANOVG_GL2_IMPLEMENTATION)
#error "ofxNanoVG: Please add one of the following definitions to your project NANOVG_GL3_IMPLEMENTATION, NANOVG_GL2_IMPLEMENTATION, NANOVG_GLES2_IMPLEMENTATION"
#endif

#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"

ofxNanoVG::~ofxNanoVG()
{
	if (!bInitialized) {
		return;
	}

	// clear added fonts
	for (Font* f: fonts) {
		delete f;
	}

#ifdef NANOVG_GL3_IMPLEMENTATION
	nvgDeleteGL3(ctx);
#elif defined NANOVG_GL2_IMPLEMENTATION
	nvgDeleteGL2(ctx);
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
#elif NANOVG_GL2_IMPLEMENTATION
	ctx = nvgCreateGL2(NVG_ANTIALIAS | (stencilStrokes?NVG_STENCIL_STROKES:0) | (debug?NVG_DEBUG:0));
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

void ofxNanoVG::followPolyline(const ofPolyline &line) {
	if (line.size() == 0) {
		return;
	}

	const vector<ofPoint>& verts = line.getVertices();
	nvgMoveTo(ctx, verts[0].x, verts[0].y);
	for (int i=1; i<verts.size(); i++) {
		nvgLineTo(ctx, verts[i].x, verts[i].y);
	}
}

void ofxNanoVG::followPolylineDashed(const ofPolyline &line, float onpx, float offpx) {
	if (line.size() == 0) {
		return;
	}

	float length = line.getPerimeter();
	for (float t=0; t<=length-onpx; t+=onpx+offpx) {
		ofVec2f p1 = line.getPointAtLength(t);
		ofVec2f p2 = line.getPointAtLength(t+onpx);
		nvgMoveTo(ctx, p1.x, p1.y);
		nvgLineTo(ctx, p2.x, p2.y);
	}
}

void ofxNanoVG::followPath(const ofPath& path, float x, float y) {
	if (x!=0 || y!=0) {
		nvgTranslate(ctx, x, y);
	}

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
	
	if (x!=0 || y!=0) {
		nvgTranslate(ctx, -x, -y);
	}
}

/******
 * For convenience
 */

void ofxNanoVG::strokeLine(float x1, float y1, float x2, float y2, const ofColor& c, float width) {
	beginPath();
	setStrokeColor(c);
	setStrokeWidth(width);
	line(x1, y1, x2, y2);
	strokePath();
}

void ofxNanoVG::strokeRect(float x, float y, float w, float h, const ofColor& c, float width) {
	beginPath();
	setStrokeColor(c);
	setStrokeWidth(width);
	rect(x, y, w, h);
	strokePath();
}

void ofxNanoVG::fillRect(float x, float y, float w, float h, const ofColor& c) {
	beginPath();
	setFillColor(c);
	rect(x, y, w, h);
	fillPath();
}

void ofxNanoVG::strokeRoundedRect(float x, float y, float w, float h, float r, const ofColor& c, float width) {
	beginPath();
	setStrokeColor(c);
	setStrokeWidth(width);
	roundedRect(x, y, w, h, r);
	strokePath();
}

void ofxNanoVG::fillRoundedRect(float x, float y, float w, float h, float r, const ofColor& c) {
	beginPath();
	setFillColor(c);
	roundedRect(x, y, w, h, r);
	fillPath();
}

void ofxNanoVG::strokeRoundedRect(float x, float y, float w, float h, float r_tl, float r_tr, float r_br, float r_bl, const ofColor& c, float width) {
	beginPath();
	setStrokeColor(c);
	setStrokeWidth(width);
	roundedRect(x, y, w, h, r_tl, r_tr, r_br, r_bl);
	strokePath();
}

void ofxNanoVG::fillRoundedRect(float x, float y, float w, float h, float r_tl, float r_tr, float r_br, float r_bl, const ofColor& c) {
	beginPath();
	setFillColor(c);
	roundedRect(x, y, w, h, r_tl, r_tr, r_br, r_bl);
	fillPath();
}

void ofxNanoVG::strokeEllipse(float cx, float cy, float rx, float ry, const ofColor& c, float width) {
	beginPath();
	setStrokeColor(c);
	setStrokeWidth(width);
	ellipse(cx, cy, rx, ry);
	strokePath();
}

void ofxNanoVG::fillEllipse(float cx, float cy, float rx, float ry, const ofColor& c) {
	beginPath();
	setFillColor(c);
	ellipse(cx, cy, rx, ry);
	fillPath();
}

void ofxNanoVG::strokeCircle(float cx, float cy, float r, const ofColor& c, float width) {
	beginPath();
	setStrokeColor(c);
	setStrokeWidth(width);
	circle(cx, cy, r);
	strokePath();
}

void ofxNanoVG::fillCircle(float cx, float cy, float r, const ofColor& c) {
	beginPath();
	setFillColor(c);
	circle(cx, cy, r);
	fillPath();
}

void ofxNanoVG::strokeArc(float cx, float cy, float r, float a0, float a1, int dir, const ofColor& c, float width)
{
	beginPath();
	setStrokeColor(c);
	setStrokeWidth(width);
	arc(cx, cy, r, a0, a1, dir);
	strokePath();
}

void ofxNanoVG::fillArc(float cx, float cy, float r, float a0, float a1, int dir, const ofColor& c)
{
	beginPath();
	setFillColor(c);
	arc(cx, cy, r, a0, a1, dir);
	strokePath();
}

void ofxNanoVG::strokePolyline(const ofPolyline &line, const ofColor &c, float width) {
	beginPath();
	setStrokeColor(c);
	setStrokeWidth(width);
	followPolyline(line);
	strokePath();
}

void ofxNanoVG::fillPolyline(const ofPolyline &line, const ofColor &c) {
	beginPath();
	setFillColor(c);
	followPolyline(line);
	fillPath();
}

/******
 * Style
 */

NVGpaint ofxNanoVG::getTexturePaint(const ofTexture& tex)
{
	if (tex.getTextureData().textureTarget != GL_TEXTURE_2D) {
		ofLogError("ofxNanoVG") << "texture target should be GL_TEXTURE_2D";
		return NVGpaint();
	}
	
	int image = nvglCreateImageFromHandle(ctx, tex.getTextureData().textureID, tex.getWidth(), tex.getHeight(), 0);
	if (image <= 0) {
		ofLogError("ofxNanoVG") << "error uploading image to NanoVG";
		return NVGpaint();
	}
	
	return nvgImagePattern(ctx, -tex.getWidth()/2, -tex.getHeight()/2, tex.getWidth(), tex.getHeight(), 0, image, 1);
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

float ofxNanoVG::drawText(const string &fontName, float x, float y, const string &text, float fontSize)
{
	Font* font = getFont(fontName);
	if (font == NULL) {
		ofLogError("ofxNanoVG::drawText", "cannot find font: %s", fontName.c_str());
		return 0;
	}

	return drawText(font, x, y, text, fontSize);
}

float ofxNanoVG::drawText(ofxNanoVG::Font *font, float x, float y, const string &text, float fontSize)
{
	if (font == NULL) {
		ofLogError("ofxNanoVG::drawText", "font == NULL");
		return 0;
	}

	nvgFontFaceId(ctx, font->id);
	nvgTextLetterSpacing(ctx, font->letterSpacing);
	nvgFontSize(ctx, fontSize);

	return nvgText(ctx, x, y, text.c_str(), NULL);
}

void ofxNanoVG::drawTextBox(const string &fontName, float x, float y, const string &text, float fontSize, float breakRowWidth, float lineHeight)
{
	Font* font = getFont(fontName);
	if (font == NULL) {
		ofLogError("ofxNanoVG::drawTextBox", "cannot find font: %s", fontName.c_str());
		return;
	}

	drawTextBox(font, x, y, text, fontSize, breakRowWidth, lineHeight);
}

void ofxNanoVG::drawTextBox(ofxNanoVG::Font *font, float x, float y, const string &text, float fontSize, float breakRowWidth, float lineHeight)
{
	if (font == NULL) {
		ofLogError("ofxNanoVG::drawTextBox", "font == NULL");
		return;
	}

	nvgFontFaceId(ctx, font->id);
	nvgTextLetterSpacing(ctx, font->letterSpacing);
	nvgTextLineHeight(ctx, lineHeight==-1?font->lineHeight:lineHeight);
	nvgFontSize(ctx, fontSize);

	nvgTextBox(ctx, x, y, breakRowWidth, text.c_str(), NULL);
}

float ofxNanoVG::drawTextOnArc(const string &fontName, float cx, float cy, float radius, float startAng, int dir, float spacing, const string &text, float fontSize, bool justMeasure)
{
	Font* font = getFont(fontName);
	if (font == NULL) {
		ofLogError("ofxNanoVG::drawTextOnArc", "cannot find font: %s", fontName.c_str());
		return 0;
	}
	
	nvgFontFaceId(ctx, font->id);
	nvgTextLetterSpacing(ctx, font->letterSpacing);
	nvgTextLineHeight(ctx, font->lineHeight);
	nvgFontSize(ctx, fontSize);
	nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_BASELINE);

	float bounds[4];
	nvgSave(ctx);
	nvgTranslate(ctx, cx, cy);
	float angle=startAng;
	for (int i=0; i<text.length(); i++) {
		if (text[i] == ' ') {
			string space = "X";
			nvgTextBounds(ctx, 0, 0, space.c_str(), space.c_str()+1, bounds);
			angle += ((dir==1)?1:-1)*ofRadToDeg((bounds[2]+spacing)/radius);
			continue;
		}
		else {
			nvgTextBounds(ctx, 0, 0, text.c_str()+i, text.c_str()+i+1, bounds);
		}
		
		if (!justMeasure) {
			nvgSave(ctx);
			nvgRotate(ctx, ofDegToRad((angle+(bounds[2]/2/radius)) + ((dir==-1)?180:0)));
			nvgTranslate(ctx, 0, (dir==1)?-radius:radius);
			nvgText(ctx, 0, 0, text.c_str()+i, text.c_str()+i+1);
			nvgRestore(ctx);
		}
		angle += ((dir==1)?1:-1)*ofRadToDeg((bounds[2]+spacing)/radius);
	}
	nvgRestore(ctx);
	return angle-startAng;
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

ofRectangle ofxNanoVG::getTextBoxBounds(const string &fontName, float x, float y, const string &text, float fontSize, float breakRowWidth, float lineHeight)
{
	Font* font = getFont(fontName);
	if (font == NULL) {
		ofLogError("ofxNanoVG::getTextBoxBounds", "cannot find font: %s", fontName.c_str());
		return ofRectangle();
	}

	return getTextBoxBounds(font, x, y, text, fontSize, breakRowWidth, lineHeight);
}

ofRectangle ofxNanoVG::getTextBoxBounds(ofxNanoVG::Font *font, float x, float y, const string &text, float fontSize, float breakRowWidth, float lineHeight)
{
	if (font == NULL) {
		ofLogError("ofxNanoVG::getTextBoxBounds", "font == NULL");
		return ofRectangle();
	}

	nvgFontFaceId(ctx, font->id);
	nvgTextLetterSpacing(ctx, font->letterSpacing);
	nvgTextLineHeight(ctx, lineHeight==-1?font->lineHeight:lineHeight);
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

void ofxNanoVG::followSvg(NSVGimage* svg, float x, float y)
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
			for (int i=0; i<path->npts; i++) {
				if (i==0) {
					moveTo(path->pts[i*2], path->pts[i*2+1]);
				}
				else {
					lineTo(path->pts[i*2], path->pts[i*2+1]);
				}
			}
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
		skew.y *= -1;
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

void ofxNanoVG::translateMatrix(float x, float y)
{
	nvgTranslate(ctx, x, y);
}

void ofxNanoVG::enableScissor(float x, float y, float w, float h)
{
	if (!bInitialized) {
		return;
	}

	nvgScissor(ctx, x, y, w, h);
}

void ofxNanoVG::disableScissor()
{
	if (!bInitialized) {
		return;
	}

	nvgResetScissor(ctx);
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
