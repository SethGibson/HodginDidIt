#include "cinder/app/AppNative.h"
#include "cinder/Camera.h"
#include "cinder/MayaCamUI.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/Timeline.h"
#include "util_pipeline.h"

using namespace ci;
using namespace ci::app;
using namespace std;

const int S_HELLO_START = 0;
const int S_ASK_START = 90;
const int S_FADE_START = 120;
const int S_FADETIME = 180;
const float S_BLEND_MAX = 1.0f;

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

	void updateFeeds();	//update blur amount and saturation amount
	void drawFeeds(); //drawcurrent state

	Vec3f phUnproject(const Matrix44f pModelView, const Matrix44f pProjection, const Area pViewport, const Vec3f pPt);
	Vec3f phDepthToWorld(const Vec3f pPt, const Area pViewport);
	Vec3f niDepthToWorld(const Vec3f pPt, const Vec2f pFactors, const Vec2f pImageRes);

	CameraPersp mCamera;
	Matrix44f mMatrixMV;
	Matrix44f mMatrixProj;
	Area mAreaView;

	float mBlendAmt;
	int mTime;
	Surface8u mSurfDepth;
	Surface8u mSurfRgb;
	Channel mChanBW;
	gl::Texture mTexRgb;
	

	UtilPipeline mPXC;
	pxcU32 mRgbW, mRgbH, mDepthW, mDepthH;

	string mHello;
	string mQuestion;
	char mCursor;
};

void HodginDidItApp::prepareSettings(Settings *pSettings)
{
	pSettings->setWindowSize(640,480);
	pSettings->setFrameRate(30);
}

void HodginDidItApp::setup()
{
	setupCiCamera();
	setupIO();
	setupGraphics();
}

void HodginDidItApp::update()
{
	if(mPXC.AcquireFrame(true))
	{
		updateFeeds();
		mPXC.ReleaseFrame();
	}
}

void HodginDidItApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
	drawFeeds();
}

void HodginDidItApp::setupCiCamera()
{
	mCamera = CameraPersp(getWindowWidth(), getWindowHeight(), 74.0f);
	mCamera.setPerspective(58.0f, 4.0f/3.0f, 0.1f,1000.0f);
	mCamera.setEyePoint(Vec3f::zero());
	mCamera.lookAt(Vec3f(0,0,100));
	mMatrixMV = mCamera.getModelViewMatrix();
	mMatrixProj = mCamera.getProjectionMatrix();
	mAreaView = gl::getViewport();
}

void HodginDidItApp::keyDown(KeyEvent pEvent)
{
	float mBlendAmt = S_BLEND_MAX;
}

void HodginDidItApp::setupIO()
{
	mPXC.EnableGesture();
	mPXC.EnableImage(PXCImage::COLOR_FORMAT_RGB24);
	mPXC.EnableImage(PXCImage::COLOR_FORMAT_DEPTH);
	mPXC.EnableImage(PXCImage::COLOR_FORMAT_VERTICES);
	mPXC.Init();

	mPXC.QueryImageSize(PXCImage::IMAGE_TYPE_COLOR, mRgbW, mRgbH);
	mPXC.QueryImageSize(PXCImage::IMAGE_TYPE_DEPTH, mDepthW, mDepthH);
}

void HodginDidItApp::setupGraphics()
{
	mSurfRgb = Surface8u(mRgbW, mRgbH, false, SurfaceChannelOrder::BGR);
	mSurfDepth = Surface8u(mDepthW, mDepthH, false, SurfaceChannelOrder::RGB);
	mBlendAmt = S_BLEND_MAX;
	mHello = "Hello.";
	mQuestion = "Are You There?";
	mCursor = '_';
}

void HodginDidItApp::updateFeeds()
{
	PXCImage *rgb = mPXC.QueryImage(PXCImage::IMAGE_TYPE_COLOR);
	PXCImage::ImageData rgbData;
	if(rgb->AcquireAccess(PXCImage::ACCESS_READ, &rgbData)>=PXC_STATUS_NO_ERROR)
	{
		mTexRgb = gl::Texture(rgbData.planes[0], GL_BGR, mRgbW, mRgbH);
		mSurfRgb = Surface8u(mTexRgb);
		mChanBW = Channel(mSurfRgb);
		rgb->ReleaseAccess(&rgbData);

		if(mTime<180)
			mBlendAmt = 1.0f-((getElapsedFrames()%S_FADETIME)/(float)S_FADETIME);
		else
			mBlendAmt = 0;
	}
}

void HodginDidItApp::drawFeeds()
{
	gl::draw(mTexRgb, Vec2f::zero());
	if(getElapsedFrames()<180)
	{
		if(mBlendAmt>0)
		{
			gl::enableAlphaBlending();
			gl::color(ColorA(1,1,1,mBlendAmt));
			gl::draw(gl::Texture(mChanBW), Vec2f::zero());
			gl::disableAlphaBlending();
		}
	}
}

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vec3f HodginDidItApp::phUnproject(Matrix44f pModelView, Matrix44f pProjection, Area pViewport, const Vec3f pPt)
{
	Matrix44f iMVP = pProjection*pModelView;
	iMVP.invert(0.0f);

	Vec4f cNormalized;
	cNormalized.x = (pPt.x - pViewport.getX1())/pViewport.getWidth()*2.0f-1.0f;
	cNormalized.y = (pPt.y - pViewport.getY1())/pViewport.getHeight()*2.0f-1.0f;
	cNormalized.z = 2.0f * pPt.z - 1.0f;
	cNormalized.w = 1.0f;

	Vec4f cObjPos = iMVP*cNormalized;
	if(cObjPos.w!=0.0f)
		cObjPos.w = 1.0f/cObjPos.w;

	return Vec3f(cObjPos.x*cObjPos.w, cObjPos.y*cObjPos.w, cObjPos.z*cObjPos.w);

}
Vec3f HodginDidItApp::phDepthToWorld(const Vec3f pPt, const Area pViewport)
{
	Vec3f retVal = Vec3f(pPt);
	
	//retVal.y = pViewport.getHeight()-retVal.y;

	retVal.z = 0.0f;
	Vec3f cNear = phUnproject(mMatrixMV, mMatrixProj, mAreaView, retVal);
	retVal.z = 1.0f;
	Vec3f cFar = phUnproject(mMatrixMV, mMatrixProj, mAreaView, retVal);

	float cProject = (pPt.z-cNear.z)/(cFar.z-cNear.z);
	retVal.x = cNear.x+cProject*(cFar.x-cNear.x);
	retVal.y = cNear.y+cProject*(cFar.y-cNear.y);
	retVal.z = pPt.z;

	return retVal;
}

Vec3f HodginDidItApp::niDepthToWorld(const Vec3f pPt, const Vec2f pFactors, const Vec2f pImageRes)
{
	Vec2f cNorm = Vec2f(pPt.x/pImageRes.x-0.5f,0.5f-pPt.y/pImageRes.y);
	Vec2f cWorld = Vec2f(cNorm.x*pPt.z*pFactors.x,cNorm.y*pPt.z*pFactors.y);

	return Vec3f(cWorld.x, cWorld.y, pPt.z);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////

CINDER_APP_NATIVE( HodginDidItApp, RendererGl )

