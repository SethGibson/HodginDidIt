#include "HodginDidItApp.h"

const int S_HELLO_START = 90;
const int S_ASK_START = 210;
const int S_BL_2_SEG = 330;
const int S_SEG_2_BW = 420;
const int S_BW_2_RGB = 510;
const int S_VEIL_START = 630;
const int S_FADE_TIME = 90;
const float S_BLEND_MAX = 1.0f;

void HodginDidItApp::prepareSettings(Settings *pSettings)
{
	pSettings->setWindowSize(640,480);
	pSettings->setFrameRate(30);
}

void HodginDidItApp::setup()
{
	setupIO();
	setupCiCamera();
	setupGraphics();
	setupShaders();
	setupFbos();
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
	else if(cTime>=S_BW_2_RGB&&cTime<S_VEIL_START)
		mStage = 5;
	else if(cTime>=S_VEIL_START)
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
			updateVeil();
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
			drawVeil();
			break;
		}
	}
}

void HodginDidItApp::setupCiCamera()
{
	PXCPointF32 cFOV;
	mPXC.QueryCapture()->QueryDevice()->QueryPropertyAsPoint(PXCCapture::Device::PROPERTY_DEPTH_FOCAL_LENGTH,&cFOV);
	mFOV.x = math<float>::atan(mDepthW/(cFOV.x*2))*2;
	mFOV.y = math<float>::atan(mDepthH/(cFOV.y*2))*2;
	mNIFactors.x = math<float>::tan(mFOV.x/2.0f)*2.0f;
	mNIFactors.y = math<float>::tan(mFOV.y/2.0f)*2.0f;
	
	float cVFov = (float)toDegrees(mFOV.y);
	float cAspect = getWindowAspectRatio();
	
	mCamera.setPerspective(cVFov,cAspect,0.1f,100.0f);
	mCamera.lookAt(Vec3f(0,0,0), Vec3f::zAxis(), Vec3f::yAxis());
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
	mDepthBuffer = new uint16_t[mDepthW*mDepthH];
	mSurfDepth = Surface8u(mDepthW, mDepthH, true, SurfaceChannelOrder::RGBA);
	mBlendAmt = S_BLEND_MAX;

	mFont = gl::TextureFont::create(Font(loadAsset("IntelClear_RG.ttf"), 48));
	mHello = "Hello.";
	mQuestion = "Are You There?";
	mCursor = '_';
	mScreenText.clear();

	mTexBg = gl::Texture(loadImage(loadAsset("bg_test.png")));
	mPixelSize = Vec2f::one()/Vec2f(160,120);
}

//global
void HodginDidItApp::updateCamera()
{
	mImgRgb = mPXC.QueryImage(PXCImage::IMAGE_TYPE_COLOR);
	if(mImgRgb->AcquireAccess(PXCImage::ACCESS_READ, &mDataRgb)>=PXC_STATUS_NO_ERROR)
	{
		mTexRgb = gl::Texture(mDataRgb.planes[0],GL_BGR,mRgbW,mRgbH);
		mImgRgb->ReleaseAccess(&mDataRgb);
	}

	if(mStage==3||mStage==4)	//BL_2_SEG
	{
		PXCImage *cImgSeg = mPXC.QuerySegmentationImage();
		PXCImage::ImageData cDataSeg;
		if(cImgSeg->AcquireAccess(PXCImage::ACCESS_READ, &cDataSeg)>=PXC_STATUS_NO_ERROR)
		{
			mTexSeg = gl::Texture(cDataSeg.planes[0],GL_LUMINANCE,mDepthW,mDepthH);
			cImgSeg->ReleaseAccess(&cDataSeg);
		}
		mChanBW = Channel(mTexRgb);
	}
	else if(mStage==4||mStage==5)	//SEG_2_BW
	{
		mChanBW = Channel(mTexRgb);
	}
	else if(mStage==5||mStage==6)	//BW_2_RGB
	{
		PXCImage *cImgDepth = mPXC.QueryImage(PXCImage::IMAGE_TYPE_DEPTH);
		PXCImage::ImageData cDataDepth;
		if(cImgDepth->AcquireAccess(PXCImage::ACCESS_READ, &cDataDepth)>=PXC_STATUS_NO_ERROR)
		{
			memcpy(mDepthBuffer, cDataDepth.planes[0], (size_t)(mDepthW*mDepthH*sizeof(pxcU16)));
			cImgDepth->ReleaseAccess(&cDataDepth);
		}
	}
}

void HodginDidItApp::setupShaders()
{
	try
	{
		mShaderDisplace = gl::GlslProg(loadAsset("shaders/vert_passthru.glsl"), loadAsset("shaders/frag_displace.glsl"));
	}
	catch(gl::GlslProgCompileExc e)
	{
		console() << e.what() << endl;
	}
	try
	{
		mShaderRipple = gl::GlslProg(loadAsset("shaders/vert_passthru.glsl"), loadAsset("shaders/frag_ripple.glsl"));
	}
	catch(gl::GlslProgCompileExc e)
	{
		console() << e.what() << endl;
	}
}

void HodginDidItApp::setupFbos()
{
	gl::Fbo::Format cFormat;
	cFormat.setSamples(4);
	cFormat.setCoverageSamples(8);
	cFormat.enableColorBuffer();
	cFormat.enableDepthBuffer();
	mFboPoints = gl::Fbo(getWindowWidth(), getWindowHeight(),cFormat);
	//mFboPoints.getTexture(0).setFlipped(true);

	cFormat.enableDepthBuffer(false);
	cFormat.setColorInternalFormat(GL_RGBA32F_ARB);

	mFboIndex = 0;
	for(size_t id=0;id<2;++id)
	{
		mFboWater[id] = gl::Fbo(getWindowWidth(), getWindowHeight(), cFormat);
		mFboWater[id].bindFramebuffer();
		gl::clear();
		mFboWater[id].unbindFramebuffer();
		mFboWater[id].getTexture(0).setWrap(GL_REPEAT,GL_REPEAT);
		mFboWater[id].getTexture(0).setFlipped(true);
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
	gl::pushModelView();
	gl::translate(getWindowWidth(),0);
	gl::scale(-1,1,1);
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
	gl::popModelView();
}

//Stage 6
void HodginDidItApp::updateVeil()
{
	processDepth();
	updateFboPoint();
	updateFboWater();
}

void HodginDidItApp::drawVeil()
{
	gl::clear(Color::black());

	mFboWater[mFboIndex].bindTexture(0,0);
	gl::enable(GL_TEXTURE_2D);
	mFboPoints.bindTexture(1);
	mShaderDisplace.bind();
	mShaderDisplace.uniform("buffer",0);
	mShaderDisplace.uniform("pixel",mPixelSize);
	mShaderDisplace.uniform("tex",1);
	gl::drawSolidRect(getWindowBounds());
	mFboPoints.unbindTexture();
	gl::disable(GL_TEXTURE_2D);
	mShaderDisplace.unbind();
}

void HodginDidItApp::processDepth()
{
	mPoints.clear();
	Surface::Iter sit = mSurfDepth.getIter(Area(0,0,mDepthW,mDepthH));
	while(sit.line())
	{
		while(sit.pixel())
		{
			float cDepth = (float)mDepthBuffer[sit.y()*mDepthW+sit.x()];
			sit.r() = 0;
			sit.g() = 0;
			sit.b() = 0;
			sit.a() = 0;
			if(cDepth<32000)
			{
				float cScaled = cDepth/1000.0f;
				if(cDepth<300)
				{
					uint8_t v = (uint8_t)lmap<float>(cDepth,0,300,0,255);
					sit.r() = v;
					sit.a() = v;
				}
				if(sit.x()%2==0&&sit.y()%2==0)
					mPoints.push_back(niDepthToWorld(Vec3f(sit.x(),sit.y(),1.0f*cScaled)));

			}
		}
	}
}

void HodginDidItApp::updateFboPoint()
{
	Area cVP = gl::getViewport();
	gl::setViewport(mFboPoints.getBounds());

	mFboPoints.bindFramebuffer();
	gl::disableDepthRead();
	gl::setMatricesWindow(getWindowSize());
	gl::draw(mTexBg, Vec2f::zero());

	gl::pushMatrices();
	gl::setMatrices(mCamera);
	gl::enableAdditiveBlending();
	for(auto vit=mPoints.begin();vit!=mPoints.end();++vit)
	{
		Vec3f cP = *vit;
		gl::color(ColorA(1-cP.z,1-cP.z,1,cP.z*0.2f));
		gl::drawCube(cP, Vec3f(0.008,0.008,0.008));
	}
	gl::disableAlphaBlending();
	gl::popMatrices();
	mFboPoints.unbindFramebuffer();

	gl::setViewport(cVP);
}

void HodginDidItApp::updateFboWater()
{
	gl::enable(GL_TEXTURE_2D);
	gl::color(Colorf::white());

	size_t cPong = (mFboIndex +1)%2;
	mFboWater[cPong].bindFramebuffer();	

	gl::setViewport(mFboWater[mFboIndex].getBounds());
	gl::setMatricesWindow(mFboWater[mFboIndex].getSize(), false);
	gl::clear();
	mFboWater[mFboIndex].bindTexture(); 

	mShaderRipple.bind();
	mShaderRipple.uniform("buffer",0);
	mShaderRipple.uniform("pixel",mPixelSize);
	gl::drawSolidRect(Rectf(0,0,getWindowWidth(),getWindowHeight()));
	mShaderRipple.unbind();
	mFboWater[mFboIndex].unbindTexture();
	gl::disable(GL_TEXTURE_2D);

	gl::enableAlphaBlending();
	gl::draw(gl::Texture(mSurfDepth),Rectf(0,0,getWindowWidth(),getWindowHeight()));
	gl::disableAlphaBlending();

	mFboWater[cPong].unbindFramebuffer();
	mFboIndex = cPong;
}

Vec3f HodginDidItApp::niDepthToWorld(const Vec3f pPoint)
{
	Vec2f cNorm = Vec2f(pPoint.x/mDepthW-0.5f,0.5f-pPoint.y/mDepthH);
	Vec2f cWorld = Vec2f(cNorm.x*pPoint.z*mNIFactors.x, cNorm.y*pPoint.z*mNIFactors.y);

	return Vec3f(cWorld.x, cWorld.y, pPoint.z);
}


CINDER_APP_NATIVE( HodginDidItApp, RendererGl )

