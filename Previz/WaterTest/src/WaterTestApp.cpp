#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/ImageIo.h"
#include "util_pipeline.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class WaterTestApp : public AppNative {
public:
	void prepareSettings(Settings *pSettings);
	void setup();
	void mouseDown( MouseEvent event );	
	void mouseDrag( MouseEvent event );	
	void mouseUp( MouseEvent event );	
	void update();
	void draw();

private:
	int mDebugDraw;
	uint16_t *mBufferDepth;
	Surface8u mSurfDepth;
	gl::Texture mTexSeg, mTexRgb, mTexBg;

	Vec2f mSizePixel;
	size_t mIndexFbo;
	gl::Fbo mFbos[2];
	
	Vec2i mMouse;
	bool mMouseDown;

	gl::GlslProg mShaderRipple, mShaderDisplace;

	UtilPipeline mPXC;
	pxcU32 mDepthW, mDepthH;
	
	void getDepthSurface();
	void setupShaders();
	void setupFbos();
	void updateFbos();
};

void WaterTestApp::prepareSettings(Settings *pSettings)
{
	pSettings->setWindowSize(640,480);
	pSettings->setFrameRate(30);
}

void WaterTestApp::setup()
{
	mPXC.EnableImage(PXCImage::COLOR_FORMAT_RGB24);
	mPXC.EnableImage(PXCImage::COLOR_FORMAT_DEPTH);
	mPXC.Init();

	mPXC.QueryImageSize(PXCImage::IMAGE_TYPE_DEPTH, mDepthW, mDepthH);
	mBufferDepth = new uint16_t[mDepthW*mDepthH];

	mSurfDepth = Surface8u(mDepthW, mDepthH, true, SurfaceChannelOrder::RGBA);
	mTexBg = loadImage(loadAsset("bg_img.jpg"));

	mSizePixel = Vec2f::one()/Vec2f(160,120);

	setupShaders();
	setupFbos();
	mDebugDraw = 0;
}

void WaterTestApp::mouseDown( MouseEvent event )
{
	mMouseDown = true;
	mouseDrag( event );
}

void WaterTestApp::mouseDrag( MouseEvent event )
{
	mMouse = event.getPos();
}

void WaterTestApp::mouseUp( MouseEvent event )
{
	mMouseDown = false;
}

void WaterTestApp::update()
{
	if(mPXC.AcquireFrame(true))
	{
		PXCImage *cImgRgb = mPXC.QueryImage(PXCImage::IMAGE_TYPE_COLOR);
		PXCImage::ImageData cDataRgb;
		if(cImgRgb->AcquireAccess(PXCImage::ACCESS_READ, &cDataRgb)>=PXC_STATUS_NO_ERROR)
		{
			mTexRgb = gl::Texture(cDataRgb.planes[0], GL_BGR, 640,480);
			cImgRgb->ReleaseAccess(&cDataRgb);
		}

		PXCImage *cImgDepth = mPXC.QueryImage(PXCImage::IMAGE_TYPE_DEPTH);
		PXCImage::ImageData cDataDepth;
		if(cImgDepth->AcquireAccess(PXCImage::ACCESS_READ, &cDataDepth)>=PXC_STATUS_NO_ERROR)
		{
			mBufferDepth = (uint16_t *)cDataDepth.planes[0];
			getDepthSurface();
			cImgDepth->ReleaseAccess(&cDataDepth);
		}
		mPXC.ReleaseFrame();
	}

	updateFbos();
}

void WaterTestApp::updateFbos()
{
	gl::enable(GL_TEXTURE_2D);
	gl::color(Colorf::white());

	size_t cPong = (mIndexFbo +1)%2;
	mFbos[cPong].bindFramebuffer();	

	gl::setViewport(mFbos[mIndexFbo].getBounds());
	gl::setMatricesWindow(mFbos[mIndexFbo].getSize(), false);
	gl::clear();
	mFbos[mIndexFbo].bindTexture(); 

	mShaderRipple.bind();
	mShaderRipple.uniform("buffer",0);
	mShaderRipple.uniform("pixel",mSizePixel);
	gl::drawSolidRect(Rectf(0,0,getWindowWidth(),getWindowHeight()));
	mShaderRipple.unbind();
	mFbos[mIndexFbo].unbindTexture();
	gl::disable(GL_TEXTURE_2D);

	gl::enableAlphaBlending();
	gl::draw(gl::Texture(mSurfDepth),Rectf(0,0,getWindowWidth(),getWindowHeight()));
	gl::disableAlphaBlending();

	mFbos[cPong].unbindFramebuffer();
	mIndexFbo = cPong;
}

void WaterTestApp::draw()
{
	gl::clear(Color::black());
	gl::setViewport(getWindowBounds());

	mFbos[mIndexFbo].bindTexture(0,0);
	gl::enable(GL_TEXTURE_2D);
	mTexRgb.bind(1);
	mShaderDisplace.bind();
	mShaderDisplace.uniform("buffer",0);
	mShaderDisplace.uniform("pixel",mSizePixel);
	mShaderDisplace.uniform("tex",1);

	gl::drawSolidRect(Rectf(0,0,getWindowWidth(),getWindowHeight()));
	mTexRgb.unbind();
	gl::disable(GL_TEXTURE_2D);
	mShaderDisplace.unbind();
}

void WaterTestApp::getDepthSurface()
{
	Surface8u::Iter iter = mSurfDepth.getIter(Area(0,0,mDepthW,mDepthH));
	while(iter.line())
	{
		while(iter.pixel())
		{
			iter.r() = 0;
			iter.g() = 0;
			iter.b() = 0;
			iter.a() = 0;
			uint16_t cVal = mBufferDepth[iter.y()*mDepthW+iter.x()];
			if(cVal<500)
			{
				float cColor = lmap<float>((float)cVal,0,500,255,0);
				iter.r() = (uint8_t)cColor;
				iter.a() = (uint8_t)cColor;
			}
		}
	}
}

void WaterTestApp::setupShaders()
{
	try
	{
		mShaderRipple = gl::GlslProg(loadAsset("shaders/passthruVert.glsl"), loadAsset("shaders/rippleFrag.glsl"));
	}
	catch(gl::GlslProgCompileExc ex)
	{
		console() << ex.what() << endl;
	}
	try
	{
		mShaderDisplace = gl::GlslProg(loadAsset("shaders/passthruVert.glsl"), loadAsset("shaders/displaceFrag.glsl"));
	}
	catch(gl::GlslProgCompileExc ex)
	{
		console() << ex.what() << endl;
	}
}

void WaterTestApp::setupFbos()
{
	gl::Fbo::Format cFormat;
	cFormat.enableColorBuffer(true);
	cFormat.enableDepthBuffer(false);
	cFormat.setColorInternalFormat(GL_RGBA32F_ARB);

	mIndexFbo = 0;
	for(size_t id=0;id<2;++id)
	{
		mFbos[id] = gl::Fbo(getWindowWidth(), getWindowHeight(), cFormat);
		mFbos[id].bindFramebuffer();
		gl::clear();
		mFbos[id].unbindFramebuffer();
		mFbos[id].getTexture().setWrap(GL_REPEAT,GL_REPEAT);
	}
}

CINDER_APP_NATIVE( WaterTestApp, RendererGl )
