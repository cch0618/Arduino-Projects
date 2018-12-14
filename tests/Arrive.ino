// initialize

bool is_arrived(float location[2], int target[2][2]) {
    // current
    float x = location[0];
    float y = location[1];

    // range
    float min_x = target[0][0];
    float max_x = target[0][1];
    float min_y = target[1][0];
    float max_y = target[1][1];

    // in range
    if ((x >= min_x && x <= max_x) && (y >= min_y && y <= max_y)) return true;
    return false;
}

void setup() {

}

void loop() {
    
}
