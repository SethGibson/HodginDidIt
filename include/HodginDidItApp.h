#ifndef __HODGINDIDIT_H__
#define __HODGINDIDIT_H__
#include "cinder/app/AppNative.h"
#include "cinder/Camera.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
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

private:
	void setupCiCamera();
	void setupIO();
	void setupGraphics();
	void setupShaders();
	void setupFbos();

	//update and draw methods
	//Global
	void updateCamera();

	//Stage 1, 2 - Hello. Are You There?
	void updateStrings(); 
	void drawStrings();

	//Stage 3,4,5 - I Can See You
	void updateBlend();
	void drawBlend();

	//Stage 6 - Reach In
	void updateVeil();
	void drawVeil();
	Vec3f niDepthToWorld(const Vec3f pPoint);

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
	Channel mChanBW;
	gl::Texture mTexRgb;
	gl::Texture mTexSeg;

	//Stage 6
	int mFboIndex;
	Vec2f mFOV, mNIFactors, mPixelSize;
	vector<Vec3f> mPoints;
	CameraPersp mCamera;
	Matrix44f mMatrixMV;
	Matrix44f mMatrixProj;
	Area mAreaView;
	Surface8u mSurfDepth;
	gl::Fbo mFboPoints;
	gl::Fbo mFboWater[2];
	gl::GlslProg mShaderBlur, mShaderDisplace, mShaderRipple;
};

#endif __HODGINDIDIT_H__