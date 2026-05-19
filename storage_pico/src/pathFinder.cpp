#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include "pathFinder.h"

pathFinder::pathFinder(int target, int currentPosition, int maxPosition) {
	this->mTarget = target;
	this->mCurrentPosition = currentPosition;
	this->mMaxPosition = maxPosition;
}

void pathFinder::setCurrentPosition(int pos) {
	mCurrentPosition = pos;
}
void pathFinder::setTarget(int t) {
	mTarget = t;
}

int pathFinder::getCircularPosition(int input) const {
	return (input + (mMaxPosition - 1)) % (mMaxPosition)+1;
}

float pathFinder::getClosestPosition() const {
	//return getCircularPosition(target) - getCircularPosition(currentPosition);
	//return 8 - (abs(getCircularPosition(target) - (getCircularPosition(currentPosition))));
	
	if (getCircularPosition(mTarget) == getCircularPosition(mCurrentPosition)) 
	{
		return 0;
		//ALDREADY AT TARGET!
	}

	float delta;

	if (abs(getCircularPosition(mTarget) - (getCircularPosition(mCurrentPosition))) < mMaxPosition - (abs(getCircularPosition(mTarget) - (getCircularPosition(mCurrentPosition)))))
	{
		
		delta = abs(getCircularPosition(mTarget) - getCircularPosition(mCurrentPosition));
		//DIRECT

		//RIGHT DIRECTION (+/-)
		if (getCircularPosition(mTarget) < getCircularPosition(mCurrentPosition))
		{
			delta *= -1;
		}
	}
	else if (abs(getCircularPosition(mTarget) - (getCircularPosition(mCurrentPosition))) > mMaxPosition - (abs(getCircularPosition(mTarget) - (getCircularPosition(mCurrentPosition)))))
	{
		
		delta = mMaxPosition - (abs(getCircularPosition(mTarget) - (getCircularPosition(mCurrentPosition))));
		//OVERFLOW

		//RIGHT DIRECTION (+/-)
		if (getCircularPosition(mTarget) > getCircularPosition(mCurrentPosition))
		{
			delta *= -1;
		}
	}
	else
	{
		delta = mMaxPosition / 2;
		//Distances are equal, both directions give same distance
	}

	

	return delta;
}