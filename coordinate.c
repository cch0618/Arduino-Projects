#include <stdio.h>
#include <math.h>

int cartesian_coordinate_evaluate(int limit[2][2], int point[2]) {
    int min_x = limit[0][0];
    int max_x = limit[0][1];
    int min_y = limit[1][0];
    int max_y = limit[1][1];
    
    int x = point[0];
    int y = point[1];

    if ((x >= min_x && x <= max_x) && (y >= min_y && y <= max_y)) return 1;
    else return 0;
}

int main() {
    int _range[2][2] = {{0, 10}, {0, 10}};
    int _point[2] = {12, 1};
    
    if (cartesian_coordinate_evaluate(_range, _point)) printf("Hello, world!");

    return 0;
}

