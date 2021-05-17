#include <iostream>

#include "image_export.hpp"

using namespace std;

enum PatternType
{
    T,
    E,
    O,
    RANDOM
};

class Segment
{
private:
    bool **prev_frame;

    bool** create_empty_frame();
    bool** initialize_frame(PatternType pattern);
    static bool T_condition(int global_x, int global_y, int full_frame_size);
    static bool E_condition(int global_x, int global_y, int full_frame_size);
    static bool O_condition(int global_x, int global_y, int full_frame_size);
    static bool random_condition(int global_x, int global_y, int full_frame_size);
    void save_frame();
    int count_neighbors(int x, int y);

public:
    bool **frame;
    int full_frame_size;
    int width;
    int height;
    int overlap_up;
    int overlap_down;
    int overlap_left;
    int overlap_right;
    int x;
    int y;
    int rank;

    Segment(PatternType initial_pattern, int full_frame_size, int width, int height, int overlap_up, int overlap_down, int overlap_left, int overlap_right, int x, int y, int rank);
    int full_width();
    int full_height();
    void print_full_frame();
    void export_full_frame(BMPExporter &exporter, int frame_number);
    void clean();
    void iteration(BMPExporter &exporter);
    void iteration();
    string convert_to_string();
};
