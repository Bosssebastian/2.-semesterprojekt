#ifndef PATHFINDER_H
#define PATHFINDER_H
class pathFinder {
private:
    int target;
    int currentPosition;
    int maxPosition;

public:
    pathFinder(int target, int currentPosition, int maxPosition);

    int getCircularPosition(int input) const;
    float getClosestPosition() const;
    void setCurrentPosition(int pos);
    void setTarget(int t);

    

    ~pathFinder() = default;
};


#endif