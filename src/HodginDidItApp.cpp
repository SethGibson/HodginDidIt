#include "HodginDidItApp.h"

const int S_HELLO_START = 0;
const int S_ASK_START = 90;
const int S_FADE_START = 120;
const int S_FADETIME = 180;
const float S_BLEND_MAX = 1.0f;


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

	mFont = gl::TextureFont::create(Font(loadAsset("IntelClear_RG.ttf"), 48));
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

CINDER_APP_NATIVE( HodginDidItApp, RendererGl )

