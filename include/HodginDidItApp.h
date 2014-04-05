#ifndef __HODGINDIDIT_H__
#define __HODGINDIDIT_H__
#include "cinder/app/AppNative.h"
#include "cinder/Camera.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/TextureFont.h"
#include "util_pipeline.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class HodginDidItApp : public AppNative {
public:
	void setup();
	void prepareSettings(Settings *pSettings);
	void update();
	void draw();
	void keyDown(KeyEvent pEvent);

private:
	void setupCiCamera();
	void setupIO();
	void setupGraphics();

	//update and draw methods
	//Stage 1 - Hello. Are You There?
	void updateStrings(); 
	void drawStrings();

	//Stage 2 - I Can See You
	void updateFeeds();	//Stage 2
	void drawFeeds(); //drawcurrent state

	//Stage 3 - Reach In
	void updateWorld(); //stage 3
	void drawWorld(); //stage 3
	
	//Stage 1 
	gl::Texture mTexRgb;
	gl::TextureFontRef mFont;
	string mHello;
	string mQuestion;
	char mCursor;

	//Stage 2
	float mBlendAmt;
	Surface8u mSurfDepth;
	Surface8u mSurfRgb;
	Channel mChanBW;

	//Stage 3
	CameraPersp mCamera;
	Matrix44f mMatrixMV;
	Matrix44f mMatrixProj;
	Area mAreaView;

	UtilPipeline mPXC;
	pxcU32 mRgbW, mRgbH, mDepthW, mDepthH;
};

#endif __HODGINDIDIT_H__