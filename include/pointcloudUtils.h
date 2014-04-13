#ifndef __PCUTILS_H__
#define __PCUTILS_H__
#include "cinder/app/AppNative.h"

using namespace ci;

namespace PointCloud
{
	Vec3f phUnproject(const Matrix44f pModelView, const Matrix44f pProjection, const Area pViewport, const Vec3f pPt);
	Vec3f phDepthToWorld(const Matrix44f pModelView, const Matrix44f pProjection, const Area pViewport, const Vec3f pPt);
	Vec3f niDepthToWorld(const Vec3f pPt, const Vec2f pFactors, const Vec2f pImageRes);
}

#endif
