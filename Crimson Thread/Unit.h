#ifndef UNIT_H
#define UNIT_H

class Unit {
private:
    int x;
    int y;
    int currentDirection; // 0: North, 1: South, 2: East, 3: West
    char standOn;

public:
    Unit(int startX, int startY, char standOn);

    int getX() const;
    int getY() const;
    void move(char** grid);
};

#endif // UNIT_H