#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "cinder/Timeline.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class TimelineTestApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();

	Anim<Color> mBgColor;
	Anim<Vec2f> mCenter;
	Anim<float> mRadius;
};

void TimelineTestApp::setup()
{
	mBgColor = Color( CM_HSV, randFloat(), 1.0f, 1.0f);
	mCenter = getWindowCenter();
	mRadius = randFloat(20,100);

	Color cBgColor = Color( CM_HSV, randFloat(), 1.0f, 1.0f);
	timeline().apply(&mBgColor, cBgColor, 2.0f, EaseInCubic());
	timeline().apply(&mCenter, Vec2f(randFloat(getWindowWidth()),randFloat(getWindowHeight())), 1.0f, EaseInCirc());
	timeline().apply(&mRadius, randFloat(20,100),1.0f,EaseInQuad());
}

void TimelineTestApp::mouseDown( MouseEvent event )
{
}

void TimelineTestApp::update()
{
	if(getElapsedFrames()%120==0)
	{
		Color cBgColor = Color( CM_HSV, randFloat(), 1.0f, 1.0f);
		timeline().appendTo(&mBgColor, cBgColor, 2.0f, EaseInCubic());
		timeline().appendTo(&mCenter, Vec2f(randFloat(getWindowWidth()),randFloat(getWindowHeight())), 1.0f, EaseInCirc());
		timeline().appendTo(&mRadius, randFloat(20,100),1.0f,EaseInQuad());
	}
}

void TimelineTestApp::draw()
{
	gl::clear( mBgColor.value() ); 
	gl::drawSolidCircle( mCenter.value(), mRadius.value());
}

CINDER_APP_NATIVE( TimelineTestApp, RendererGl )
