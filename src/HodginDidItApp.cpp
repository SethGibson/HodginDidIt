#include "cinder/app/AppNative.h"
#include "cinder/Camera.h"
#include "cinder/MayaCamUI.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
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
	Vec3f phUnproject(const Matrix44f pModelView, const Matrix44f pProjection, const Area pViewport, const Vec3f pPt);
	Vec3f phDepthToWorld(const Vec3f pPt, const Area pViewport);
	Vec3f niDepthToWorld(const Vec3f pPt, const Vec2f pFactors, const Vec2f pImageRes);

	CameraPersp mCamera;
	Matrix44f mMvM;
	Matrix44f mProjM;
	Area mViewport;

	UtilPipeline mPXC;
};

void HodginDidItApp::prepareSettings(Settings *pSettings)
{
	pSettings->setWindowSize(640,480);
	pSettings->setFrameRate(30);
}

void HodginDidItApp::setup()
{
	mCamera = CameraPersp(getWindowWidth(), getWindowHeight(), 74.0f);
	mCamera.setPerspective(58.0f, 4.0f/3.0f, 0.1f,1000.0f);
	mCamera.setEyePoint(Vec3f::zero());
	mCamera.lookAt(Vec3f(0,0,100));
	mMvM = mCamera.getModelViewMatrix();
	mProjM = mCamera.getProjectionMatrix();
	mViewport = gl::getViewport();

	mPXC.EnableGesture();
	mPXC.EnableImage(PXCImage::COLOR_FORMAT_RGB32);
	mPXC.EnableImage(PXCImage::COLOR_FORMAT_DEPTH);
	mPXC.EnableImage(PXCImage::COLOR_FORMAT_VERTICES);
	mPXC.Init();
}

void HodginDidItApp::update()
{

}

void HodginDidItApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
	gl::setMatrices( mCamera );
	gl::drawCube(Vec3f(0,0,10), Vec3f(10,10,10));

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
	Vec3f cNear = phUnproject(mMvM, mProjM, mViewport, retVal);
	retVal.z = 1.0f;
	Vec3f cFar = phUnproject(mMvM, mProjM, mViewport, retVal);

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

CINDER_APP_NATIVE( HodginDidItApp, RendererGl )

