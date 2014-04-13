#include "HdiVeil.h"
#include "cinder/Rand.h"
namespace Veil
{
//VLPoint
VLPoint::VLPoint()
{
}

VLPoint::VLPoint(Vec2f pPosInit, float pMass, float pRadius):mPosInit(pPosInit),mPos(pPosInit),mMass(pMass),mRadius(pRadius)
{
}

VLPoint::~VLPoint()
{
}

void VLPoint::step(float pStep)
{
	float cX = mRadius*math<float>::cos(pStep*mMass)+mPosInit.x;
	float cY = mRadius*math<float>::sin(pStep*mMass)+mPosInit.y;

	mPos.set(cX, cY);
}

void VLPoint::display()
{
	gl::drawSolidCircle(mPos,2);
}

void VLPoint::getPos(float &pX, float &pY)
{
	pX = mPos.x;
	pY = mPos.y;
}

//VLForce
VLForce::VLForce()
{
}

VLForce::VLForce(Vec2f pCenter, Vec2f pRadius):mCenter(pCenter),mRadius(pRadius)
{
}

VLForce::~VLForce()
{
}

void VLForce::apply()
{
}

//VLVeil
VLVeil::VLVeil()
{
}

VLVeil::VLVeil(int pCount)
{
	addParticles(pCount);
	mXVals = new float[pCount];
	mYVals = new float[pCount];
}

VLVeil::~VLVeil()
{
}

void VLVeil::addParticle()
{
	Vec2f cPos0 = Vec2f(randFloat(10,630), randFloat(10,470));
	float cMass = randFloat(0.01f,0.1f);
	float cRadius = randFloat(10,50);
	mParticles.push_back(VLPoint(cPos0, cMass, cRadius));
}

void VLVeil::addParticles(int pCount)
{
	for(int i=0;i<pCount;++i)
		addParticle();
}

void VLVeil::setForce(VLForce pForce)
{
}

void VLVeil::step(float pStep)
{
	for(unsigned int pi=0;pi<mParticles.size();++pi)
	{
		mParticles[pi].step(pStep+pi);
		mParticles[pi].getPos(mXVals[pi], mYVals[pi]);
	}
}

void VLVeil::display()
{
	gl::color(Color(1,1,0));
	for(auto it=mParticles.begin();it!=mParticles.end();++it)
		it->display();
	gl::color(Color::white());
}

}	//namespace Veil