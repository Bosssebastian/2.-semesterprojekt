#ifndef PATHFINDER_H
#define PATHFINDER_H
class pathFinder {
private:
    int mTarget;
    int mCurrentPosition;
    int mMaxPosition;

public:
    pathFinder(int target, int currentPosition, int maxPosition);

    int getCircularPosition(int input) const;
    float getClosestPosition() const;
    void setCurrentPosition(int pos);
    void setTarget(int t);

    

    ~pathFinder() = default;
};


#endif