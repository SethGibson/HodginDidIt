#include "HodginDidItApp.h"

const int S_HELLO_START = 90;
const int S_ASK_START = 210;
const int S_BL_2_SEG = 330;
const int S_SEG_2_BW = 420;
const int S_BW_2_RGB = 510;
const int S_WORLD_START = 630;
const int S_FADE_TIME = 90;
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
	else if(cTime>=S_HELLO_START&&cTime<S_ASK_START)
		mStage = 1;
	else if(cTime>=S_ASK_START&&cTime<S_BL_2_SEG)
		mStage = 2;
	else if(cTime>=S_BL_2_SEG&&cTime<S_SEG_2_BW)
		mStage = 3;
	else if(cTime>=S_SEG_2_BW&&cTime<S_BW_2_RGB)
		mStage = 4;
	else if(cTime>=S_BW_2_RGB&&cTime<S_WORLD_START)
		mStage = 5;
	else if(cTime>=S_WORLD_START)
		mStage = 6;
		
	if(mStage>=3)
	{
		if(mPXC.AcquireFrame(true))
		{
			updateCamera();
			mPXC.ReleaseFrame();
		}
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
		case 4:
		case 5:
		{
			updateBlend();
			break;
		}
		case 6:
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
		case 4:
		case 5:
		{
			drawBlend();
			break;
		}
		case 6:
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
	mPXC.EnableImage(PXCImage::COLOR_FORMAT_RGB24);
	mPXC.EnableImage(PXCImage::COLOR_FORMAT_DEPTH);
	mPXC.EnableImage(PXCImage::COLOR_FORMAT_VERTICES);
	mPXC.EnableSegmentation();
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
	mImgRgb = mPXC.QueryImage(PXCImage::IMAGE_TYPE_COLOR);
	if(mImgRgb->AcquireAccess(PXCImage::ACCESS_READ, &mDataRgb)>=PXC_STATUS_NO_ERROR)
	{
		mTexRgb = gl::Texture(mDataRgb.planes[0],GL_BGR,mRgbW,mRgbH);
		mChanBW = Channel(mTexRgb);
		mImgRgb->ReleaseAccess(&mDataRgb);
	}

	if(mStage==3||mStage==4)	//BL_2_SEG
	{
		PXCImage *mImgSeg = mPXC.QuerySegmentationImage();
		PXCImage::ImageData mDataSeg;
		if(mImgSeg->AcquireAccess(PXCImage::ACCESS_READ, &mDataSeg)>=PXC_STATUS_NO_ERROR)
		{
			mTexSeg = gl::Texture(mDataSeg.planes[0],GL_LUMINANCE,mDepthW,mDepthH);
			mImgSeg->ReleaseAccess(&mDataSeg);
		}
	}
	else if(mStage==4||mStage==5)	//SEG_2_BW
	{
		// placeholder?
	}
	else if(mStage==5)	//BW_2_RGB
	{
		// placeholder?
	}

	else if(mStage==6)
	{
	}
}

//Stage 1,2
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

//Stage 3,4,5
void HodginDidItApp::updateBlend()
{
	int cTime = getElapsedFrames();
	if(cTime==S_BL_2_SEG||cTime==S_SEG_2_BW||cTime==S_BW_2_RGB)
	{
		mBlendCounter = 0;
		mBlendAmt = 1.0f;
		console() << "STAGE: " << mStage << endl;
	}
	int cTimeBase;
	switch(mStage)
	{
		case 3:
		{
			cTimeBase = S_BL_2_SEG;
			break;
		}
		case 4:
		{
			cTimeBase = S_SEG_2_BW;
			break;
		}
		case 5:
		{
			cTimeBase = S_BW_2_RGB;
			break;
		}
	}
	cTimeBase = cTime-cTimeBase;
	if(mStage<=5&&cTimeBase<90)
		mBlendAmt = lerp<float>( 0.0f, 1.0f, (cTimeBase%S_FADE_TIME)/(float)S_FADE_TIME );
	else
		mBlendAmt = 1;
	++mBlendCounter;
}

void HodginDidItApp::drawBlend()
{
	gl::clear(Color::black());
	gl::enableAlphaBlending();
	switch(mStage)
	{
		case 3:	//BL_2_SEG
		{
			gl::color(ColorA(1,1,1,mBlendAmt));
			gl::draw(mTexSeg, Rectf(0,0,640,480));
			break;
		}
		case 4:	//SEG_2_BW
		{
			gl::color(ColorA(1,1,1,1));
			gl::draw(mTexSeg, Rectf(0,0,640,480));
			gl::color(ColorA(1,1,1,mBlendAmt));
			gl::draw(gl::Texture(mChanBW), Vec2f::zero());
			break;
		}
		case 5:	//BW_2_RGB
		{
			if(mBlendAmt<1)
			{
				gl::color(ColorA(1,1,1,1));
				gl::draw(gl::Texture(mChanBW), Vec2f::zero());
				gl::color(ColorA(1,1,1,mBlendAmt));
			}
			gl::draw(mTexRgb, Rectf(0,0,640,480));
			break;
		}
	}
	gl::disableAlphaBlending();
}

//Stage 6
void HodginDidItApp::updateWorld()
{
}

void HodginDidItApp::drawWorld()
{
}

CINDER_APP_NATIVE( HodginDidItApp, RendererGl )

