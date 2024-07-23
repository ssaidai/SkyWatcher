#ifndef SKYWATCHER_STRUCTS_H
#define SKYWATCHER_STRUCTS_H

struct Position {
    double x, y, z;
};


// maybe will need to add start and end position
struct Path{
    double distance;
    float time;
};

#endif //SKYWATCHER_STRUCTS_H
