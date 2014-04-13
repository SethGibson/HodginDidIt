#include "cinder/app/AppNative.h"
#include "cinder/Camera.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"
#include "cinder/MayaCamUI.h"
#include "util_pipeline.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class PCloudApp : public AppNative {
public:
	void prepareSettings(Settings *pSettings);
	void setup();
	void update();
	void draw();

private:
	void setupCamera();
	void setupFbos();
	void setupShaders();
	void updatePointFbo();
	void updateVeilFbos();
	void applyShaders();
	void processDepth();
	void drawPoints();
	Vec3f niDepthToWorld(const Vec3f pPoint);

	int mFboIndex, mDebugMode;
	float mDebugFOV;
	Vec2f mFOV, mNIFactors, mPixelSize;
	vector<Vec3f> mPoints;
	Surface8u mSurfDepth;
	CameraPersp mCamera;
	gl::Fbo mFboPoints;
	gl::Fbo mFboVeil[2];
	gl::GlslProg mShaderRipple, mShaderDisplace;
	gl::Texture mTexBg;

	UtilPipeline mPXC;
	pxcU32 mDepthW, mDepthH;
	pxcU16 *mDepthMap;
};

void PCloudApp::prepareSettings(Settings *pSettings)
{
	pSettings->setWindowSize(640,480);
	pSettings->setFrameRate(30);
}

void PCloudApp::setup()
{
	mPXC.EnableImage(PXCImage::COLOR_FORMAT_DEPTH);
	mPXC.Init();
	mPXC.QueryImageSize(PXCImage::IMAGE_TYPE_DEPTH, mDepthW, mDepthH);

	mDepthMap = new pxcU16[mDepthW*mDepthH];	
	mSurfDepth = Surface8u(mDepthW, mDepthH, true, SurfaceChannelOrder::RGBA);
	mPixelSize = Vec2f::one()/Vec2f(160,120);
	mDebugMode = 2;
	setupCamera();
	setupFbos();
	setupShaders();

	mTexBg = gl::Texture(loadImage(loadAsset("bg_test.png")));
}

void PCloudApp::setupCamera()
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

void PCloudApp::setupFbos()
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
		mFboVeil[id] = gl::Fbo(getWindowWidth(), getWindowHeight(), cFormat);
		mFboVeil[id].bindFramebuffer();
		gl::clear();
		mFboVeil[id].unbindFramebuffer();
		mFboVeil[id].getTexture(0).setWrap(GL_REPEAT,GL_REPEAT);
		mFboVeil[id].getTexture(0).setFlipped(true);
	}
}

void PCloudApp::setupShaders()
{
	try
	{
		mShaderRipple = gl::GlslProg(loadAsset("shaders/vert_passthru.glsl"), loadAsset("shaders/frag_ripple.glsl"));
	}
	catch(gl::GlslProgCompileExc ex)
	{
		console() << ex.what() << endl;
	}
	try
	{
		mShaderDisplace = gl::GlslProg(loadAsset("shaders/vert_passthru.glsl"), loadAsset("shaders/frag_displace.glsl"));
	}
	catch(gl::GlslProgCompileExc ex)
	{
		console() << ex.what() << endl;
	}

}

void PCloudApp::processDepth()
{
	mPoints.clear();
	Surface::Iter sit = mSurfDepth.getIter(Area(0,0,mDepthW,mDepthH));
	while(sit.line())
	{
		while(sit.pixel())
		{
			float cDepth = (float)mDepthMap[sit.y()*mDepthW+sit.x()];
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

void PCloudApp::updatePointFbo()
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

void PCloudApp::updateVeilFbos()
{
	gl::enable(GL_TEXTURE_2D);
	gl::color(Colorf::white());

	size_t cPong = (mFboIndex +1)%2;
	mFboVeil[cPong].bindFramebuffer();	

	gl::setViewport(mFboVeil[mFboIndex].getBounds());
	gl::setMatricesWindow(mFboVeil[mFboIndex].getSize(), false);
	gl::clear();
	mFboVeil[mFboIndex].bindTexture(); 

	mShaderRipple.bind();
	mShaderRipple.uniform("buffer",0);
	mShaderRipple.uniform("pixel",mPixelSize);
	gl::drawSolidRect(Rectf(0,0,getWindowWidth(),getWindowHeight()));
	mShaderRipple.unbind();
	mFboVeil[mFboIndex].unbindTexture();
	gl::disable(GL_TEXTURE_2D);

	gl::enableAlphaBlending();
	gl::draw(gl::Texture(mSurfDepth),Rectf(0,0,getWindowWidth(),getWindowHeight()));
	gl::disableAlphaBlending();

	mFboVeil[cPong].unbindFramebuffer();
	mFboIndex = cPong;
}

void PCloudApp::update()
{
	
	if(mPXC.AcquireFrame(true))
	{
		PXCImage *cDepthImg = mPXC.QueryImage(PXCImage::IMAGE_TYPE_DEPTH);
		PXCImage::ImageData cDepthData;
		
		if(cDepthImg->AcquireAccess(PXCImage::ACCESS_READ, &cDepthData)>=PXC_STATUS_NO_ERROR)
		{
			memcpy(mDepthMap, cDepthData.planes[0],(size_t)(mDepthW*mDepthH*sizeof(pxcU16)));
			cDepthImg->ReleaseAccess(&cDepthData);
		}
		mPXC.ReleaseFrame();
	}
	processDepth();
	updatePointFbo();
	updateVeilFbos();
}

Vec3f PCloudApp::niDepthToWorld(const Vec3f pPoint)
{
	Vec2f cNorm = Vec2f(pPoint.x/mDepthW-0.5f,0.5f-pPoint.y/mDepthH);
	Vec2f cWorld = Vec2f(cNorm.x*pPoint.z*mNIFactors.x, cNorm.y*pPoint.z*mNIFactors.y);

	return Vec3f(cWorld.x, cWorld.y, pPoint.z);
}

void PCloudApp::draw()
{
	gl::clear(Color::black());

	mFboVeil[mFboIndex].bindTexture(0,0);
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

CINDER_APP_NATIVE( PCloudApp, RendererGl( RendererGl::AA_MSAA_32) )
