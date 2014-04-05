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
	if(cTime < S_HELLO_START)
		mStage = 0;
	else if(cTime>S_HELLO_START&&cTime<S_ASK_START)
		mStage = 1;
	else if(cTime>=S_ASK_START&&cTime<S_FADE_START)
		mStage = 2;
	/*
	else if(cTime>S_FADE_START&&cTime<S_FADE_START+S_FADETIME)
		mStage = 3;
	else
		mStage = 4;
		*/
	if(mPXC.AcquireFrame(true))
	{
		updateCamera();
		mPXC.ReleaseFrame();
	}

	switch(mStage)
	{
		case 1:
		case 2:
		{
			updateStrings();
			break;
		}
		case 3:
		{
			updateFeeds();
			break;
		}
		case 4:
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
		case 2:
		{
			drawStrings();
			break;
		}
		case 3:
		{
			drawFeeds();
			break;
		}
		case 4:
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
	mScreenText.clear();
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
	int cTime = getElapsedFrames();
	if(cTime==0||cTime==S_ASK_START)
		mScreenText="";
	if(mStage==1)
	{
		if(getElapsedFrames()%2==0)
		{
			if(mScreenText.size()<mHello.size())
				mScreenText+=mHello[mScreenText.size()];
		}
	}
	else if(mStage==2)
	{
		if(getElapsedFrames()%2==0)
		{
			if(mScreenText.size()<mQuestion.size())
				mScreenText+=mQuestion[mScreenText.size()];
		}
	}
}

void HodginDidItApp::drawStrings()
{
	Vec2f cOffset = mFont->measureString(mScreenText);
	Vec2f cCursorPos = mFont->measureString(mCursor);
	Rectf cFit = Rectf(320-(cOffset.x*0.5f),240-(cOffset.y*0.5f),320+(cOffset.x*0.5f),240+(cOffset.y*0.5f));
	Rectf cCursorRect = Rectf(cFit.x2,cFit.y1,cFit.x2+cCursorPos.x,cFit.y1+cCursorPos.y);
	mFont->drawString(mScreenText, cFit);
	if(getElapsedFrames()%10<5)
		mFont->drawString(mCursor, cCursorRect);
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

