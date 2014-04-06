#ifndef __HODGINDIDIT_H__
#define __HODGINDIDIT_H__
#include "cinder/app/AppNative.h"
#include "cinder/Camera.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/TextureFont.h"
#include "pointcloudUtils.h"
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

private:
	void setupCiCamera();
	void setupIO();
	void setupGraphics();

	//update and draw methods
	//Global
	void updateCamera();

	//Stage 1, 2 - Hello. Are You There?
	void updateStrings(); 
	void drawStrings();

	//Stage 3 - I Can See You
	void updateBlend();
	void drawBlend();

	//Stage 4 - Reach In
	void updateWorld();
	void drawWorld();

	//Global
	int mStage;
	UtilPipeline mPXC;
	pxcU32 mRgbW, mRgbH, mDepthW, mDepthH;
	PXCImage *mImgRgb;
	PXCImage::ImageData mDataRgb;

	//Stage 1, 2
	gl::TextureFontRef mFont;
	string mHello;
	string mQuestion;
	string mScreenText;
	string mCursor;

	//Stage 3,4,5
	int mBlendCounter;
	float mBlendAmt;
	Surface8u mSurfDepth;
	Surface8u mSurfRgb;
	Channel mChanBW;
	gl::Texture mTexRgb;
	gl::Texture mTexSeg;
	gl::GlslProg mShaderBlur;

	//Stage 6
	CameraPersp mCamera;
	Matrix44f mMatrixMV;
	Matrix44f mMatrixProj;
	Area mAreaView;
};

#endif __HODGINDIDIT_H__