#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include "pathFinder.h"


int target;
int currentPosition;
pathFinder::pathFinder(int target, int currentPosition, int maxPosition) {
	this->target = target;
	this->currentPosition = currentPosition;
	this->maxPosition = maxPosition;
}

void pathFinder::setCurrentPosition(int pos) {
	currentPosition = pos;
}
void pathFinder::setTarget(int t) {
	target = t;
}

int pathFinder::getCircularPosition(int input) const {
	return (input + (maxPosition - 1)) % (maxPosition)+1;
}

float pathFinder::getClosestPosition() const {
	//return getCircularPosition(target) - getCircularPosition(currentPosition);
	//return 8 - (abs(getCircularPosition(target) - (getCircularPosition(currentPosition))));
	
	if (getCircularPosition(target) == getCircularPosition(currentPosition)) 
	{
		return 0;
		//ALDREADY AT TARGET!
	}

	float delta;

	if (abs(getCircularPosition(target) - (getCircularPosition(currentPosition))) < maxPosition - (abs(getCircularPosition(target) - (getCircularPosition(currentPosition)))))
	{
		
		delta = abs(getCircularPosition(target) - getCircularPosition(currentPosition));
		//DIRECT

		//RIGHT DIRECTION (+/-)
		if (getCircularPosition(target) < getCircularPosition(currentPosition))
		{
			delta *= -1;
		}
	}
	else if (abs(getCircularPosition(target) - (getCircularPosition(currentPosition))) > maxPosition - (abs(getCircularPosition(target) - (getCircularPosition(currentPosition)))))
	{
		
		delta = maxPosition - (abs(getCircularPosition(target) - (getCircularPosition(currentPosition))));
		//OVERFLOW

		//RIGHT DIRECTION (+/-)
		if (getCircularPosition(target) > getCircularPosition(currentPosition))
		{
			delta *= -1;
		}
	}
	else
	{
		delta = getCircularPosition(target) - getCircularPosition(currentPosition);
		//Distances are equal, both directions give same distance
	}

	

	return delta;
}

