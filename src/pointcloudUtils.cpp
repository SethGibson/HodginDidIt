#include "pointcloudUtils.h"

Vec3f PointCloud::phUnproject(const Matrix44f pModelView, const Matrix44f pProjection, const Area pViewport, const Vec3f pPt)
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

Vec3f PointCloud::phDepthToWorld(const Matrix44f pModelView, const Matrix44f pProjection, const Area pViewport, const Vec3f pPt)
{
	Vec3f retVal = Vec3f(pPt);
	
	//retVal.y = pViewport.getHeight()-retVal.y;

	retVal.z = 0.0f;
	Vec3f cNear = phUnproject(pModelView, pProjection, pViewport, retVal);
	retVal.z = 1.0f;
	Vec3f cFar = phUnproject(pModelView, pProjection, pViewport, retVal);

	float cProject = (pPt.z-cNear.z)/(cFar.z-cNear.z);
	retVal.x = cNear.x+cProject*(cFar.x-cNear.x);
	retVal.y = cNear.y+cProject*(cFar.y-cNear.y);
	retVal.z = pPt.z;

	return retVal;
}

Vec3f PointCloud::niDepthToWorld(const Vec3f pPt, const Vec2f pFactors, const Vec2f pImageRes)
{
	Vec2f cNorm = Vec2f(pPt.x/pImageRes.x-0.5f,0.5f-pPt.y/pImageRes.y);
	Vec2f cWorld = Vec2f(cNorm.x*pPt.z*pFactors.x,cNorm.y*pPt.z*pFactors.y);

	return Vec3f(cWorld.x, cWorld.y, pPt.z);
}
