#ifndef __FEEDBLENDER_H__
#define __FEEDBLENDER_H__
#include "cinder/app/AppNative.h"
#include "cinder/gl/Texture.h"
#include "util_pipeline.h"

using namespace ci;

class HDIFeedBlender
{
public:
	HDIFeedBlender();
	~HDIFeedBlender();
	void update();
	void draw();
	
private:
	UtilPipeline *mPXC;
	float mBlendAmt;
	gl::Texture mTexSrc, mTexDst;
};
#endif