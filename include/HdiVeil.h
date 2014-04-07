#ifndef __HDIVEIL_H__
#define __HDIVEIL_H__
#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "VoronoiDiagramGenerator.h"

using namespace std;
using namespace ci;

namespace Veil
{
class VLPoint
{
public:
	VLPoint();
	VLPoint(Vec2f pPosInit, float pMass, float pRadius);
	~VLPoint();

	void step(float pStep);
	void display();
	void getPos(float &pX, float &pY);
private:
	bool mAffected;
	float mRadius;
	float mMass;

	Vec2f mPosInit;
	Vec2f mPos;
	Vec2f mVel;
};

class VLForce
{
public:
	VLForce();
	VLForce(Vec2f pCenter, Vec2f pRadius);
	~VLForce();

	void apply();

private:
	Vec2f mCenter;
	Vec2f mRadius;
};

class VLVeil
{
public:
	VLVeil();
	VLVeil(int pCount);
	~VLVeil();

	void setForce(VLForce pForce);
	void step(float pStep);
	void display();
	void addParticle();
	void addParticles(int pCount);

private:
	float *mXVals;
	float *mYVals;
	vector<VLPoint> mParticles;
	VLForce mForce;
	VoronoiDiagramGenerator mVoronoi;
};

}
#endif