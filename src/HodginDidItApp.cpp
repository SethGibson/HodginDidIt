#include "HodginDidItApp.h"

const int S_HELLO_START = 90;
const int S_ASK_START = 180;
const int S_FADE_START = 270;

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
	int cTime = getElapsedFrames();
	if(cTime>S_HELLO_START&&cTime<S_FADE_START)
		mStage = 1;

	if(cTime>S_FADE_START&&cTime<S_FADE_START+S_FADETIME)
		mStage = 2;

	else
		mStage = 3;

	if(mPXC.AcquireFrame(true))
	{
		updateCamera();
		mPXC.ReleaseFrame();
	}

	switch(mStage)
	{
		case 1:
		{
			updateStrings();
			break;
		}
		case 2:
		{
			updateFeeds();
			break;
		}
		case 3:
		{
			updateWorld();
			break;
		}
	}
}

void HodginDidItApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 

	switch(mStage)
	{
		case 1:
		{
			drawStrings();
			break;
		}
		case 2:
		{
			drawFeeds();
			break;
		}
		case 3:
		{
			drawWorld();
			break;
		}
	}
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

//global
void HodginDidItApp::updateCamera()
{
	PXCImage *rgb = mPXC.QueryImage(PXCImage::IMAGE_TYPE_COLOR);
	PXCImage::ImageData rgbData;
	if(rgb->AcquireAccess(PXCImage::ACCESS_READ, &rgbData)>=PXC_STATUS_NO_ERROR)
	{
		mTexRgb = gl::Texture(rgbData.planes[0], GL_BGR, mRgbW, mRgbH);
		mSurfRgb = Surface8u(mTexRgb);
		rgb->ReleaseAccess(&rgbData);
	}
}

//Stage 1
void HodginDidItApp::updateStrings()
{
}

void HodginDidItApp::drawStrings()
{
	while(1)
	{
	}
}

//Stage 2
void HodginDidItApp::updateFeeds()
{
	mChanBW = Channel(mSurfRgb);
	if(getElapsedFrames()<S_FADE_START+S_FADETIME)
		mBlendAmt = 1.0f-((getElapsedFrames()%S_FADE_START)/(float)S_FADETIME);
	else
		mBlendAmt = 0;
}

void HodginDidItApp::drawFeeds()
{
	gl::draw(mTexRgb, Vec2f::zero());
	if(mBlendAmt>0)
	{
		gl::enableAlphaBlending();
		gl::color(ColorA(1,1,1,mBlendAmt));
		gl::draw(gl::Texture(mChanBW), Vec2f::zero());
		gl::disableAlphaBlending();
	}
}

//Stage 3
void HodginDidItApp::updateWorld()
{
}

void HodginDidItApp::drawWorld()
{
}

CINDER_APP_NATIVE( HodginDidItApp, RendererGl )

