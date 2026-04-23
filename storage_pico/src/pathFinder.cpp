#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include "pathFinder.h"

pathFinder::pathFinder(int target, int currentPosition, int maxPosition) {
	this->target = target;
	this->currentPosition = currentPosition;
	this->maxPosition = maxPosition;
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

int main()
{
	while (true)
	{
		std::cout << "Enter current position: ";
		std::cin >> currentPosition;

		std::cout << std::endl;

		std::cout << "Enter the target position: ";
		std::cin >> target;

		getClosestPosition(target, currentPosition);
		std::cout << "Need to move: " << getClosestPosition(target, currentPosition) << std::endl;
	}
	
}