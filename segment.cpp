#include <random>
#include <functional>

#include "segment.hpp"

bool** Segment::create_empty_frame()
{
    bool **new_frame = new bool*[full_height()];
    for (int i = 0; i < full_height(); i++)
        new_frame[i] = new bool[full_width()];
    return new_frame;
}

bool Segment::T_condition(int global_x, int global_y, int full_frame_size)
{
    return global_y == 0 || global_x == full_frame_size / 2;
}

bool Segment::E_condition(int global_x, int global_y, int full_frame_size)
{
    return global_y == 0 || global_y == full_frame_size / 2 || global_y == full_frame_size - 1 || global_x == 0;
}

bool Segment::O_condition(int global_x, int global_y, int full_frame_size)
{
    return global_y == 0 || global_y == full_frame_size - 1 || global_x == 0 || global_x == full_frame_size - 1;
}

bool Segment::random_condition(int global_x, int global_y, int full_frame_size)
{
    static auto generator = bind(uniform_int_distribution<>(0, 1), default_random_engine());
    return generator();
}

void Segment::save_frame()
{
    for (int i = 0; i < full_height(); i++)
        for (int j = 0; j < full_width(); j++)
            prev_frame[i][j] = frame[i][j];
}

Segment::Segment(PatternType initial_pattern, int full_frame_size, int width, int height, int overlap_up, int overlap_down, int overlap_left, int overlap_right, int x, int y, int rank)
{
    this->full_frame_size = full_frame_size;
    this->width = width;
    this->height = height;
    this->overlap_up = overlap_up;
    this->overlap_down = overlap_down;
    this->overlap_left = overlap_left;
    this->overlap_right = overlap_right;
    this->x = x;
    this->y = y;
    this->rank = rank;

    this->frame = initialize_frame(initial_pattern);
    this->prev_frame = create_empty_frame();
}

int Segment::full_width()
{
    return width + overlap_left + overlap_right;
}

int Segment::full_height()
{
    return height + overlap_up + overlap_down;
}

void Segment::print_full_frame()
{
    for (int i = 0; i < full_frame_size; i++)
    {
        for (int j = 0; j < full_frame_size; j++)
        {
            if (frame[i][j])
                cout << '*';
            else
                cout << ' ';
        }
        cout << endl;
    }
}

string Segment::convert_to_string()
{
    string s = "";
    for (int i = 0; i < full_height(); i++)
    {
        for (int j = 0; j < full_width(); j++)
        {
            if (frame[i][j])
                s += '*';
            else
                s += '_';
        }
        s += '\n';
    }
    return s;
}

void Segment::export_full_frame(BMPExporter &exporter, int frame_number)
{
    for (int i = 0; i < full_frame_size; i++)
    {
        for (int j = 0; j < full_frame_size; j++)
        {
            if (frame[i][j])
                exporter.big_pixel(j, i, 255, 255, 255);
            else
                exporter.big_pixel(j, i, 0, 0, 0);
        }
    }
    exporter.write("frames/frame" + to_string(frame_number) + ".bmp");
}

bool** Segment::initialize_frame(PatternType pattern)
{
    bool (*condition)(int, int, int);
    switch (pattern)
    {
    case T:
        condition = T_condition;
        break;
    case E:
        condition = E_condition;
        break;
    case O:
        condition = O_condition;
        break;
    case RANDOM:
        condition = random_condition;
        break;
    default:
        return NULL;
    }

    bool **new_frame = create_empty_frame();
    int global_x, global_y;

    for (int i = 0; i < full_height(); i++)
    {
        global_y = i - overlap_up + y;
        for (int j = 0; j < full_width(); j++)
        {
            global_x = j - overlap_left + x;
            if (condition(global_x, global_y, full_frame_size))
                new_frame[i][j] = true;
            else
                new_frame[i][j] = false;
        }
    }
    return new_frame;
}

void Segment::clean()
{
    for (int i = 0; i < full_height(); i++)
    {
        delete [] frame[i];
        delete [] prev_frame[i];
    }
    delete [] frame;
    delete [] prev_frame;
}

int Segment::count_neighbors(int x, int y)
{
    int start_x = x - 1 > 0 ? x - 1 : 0;
    int end_x = x + 1 < full_width() ? x + 1 : full_width() - 1;
    int start_y = y - 1 > 0 ? y - 1 : 0;
    int end_y = y + 1 < full_height() ? y + 1 : full_height() - 1;
    int neighbors = 0;
    for (int i = start_y; i <= end_y; i++)
        for (int j = start_x; j <= end_x; j++)
            if ((j != x || i != y) && prev_frame[i][j])
                neighbors++;
    return neighbors;
}

void Segment::iteration(BMPExporter &exporter)
{
    int neighbors;
    save_frame();
    for (int i = overlap_up; i < full_height() - overlap_down; i++)
    {
        for (int j = overlap_left; j < full_width() - overlap_right; j++)
        {
            neighbors = count_neighbors(j, i);
            if (prev_frame[i][j])
            {
                exporter.big_pixel(j - overlap_left, i - overlap_up, 255, 255, 255);
                if (neighbors > 3 || neighbors < 2)
                    frame[i][j] = false;
            }
            else
            {
                exporter.big_pixel(j - overlap_left, i - overlap_up, 0, 0, 0);
                if (neighbors == 3)
                    frame[i][j] = true;
            }
        }
    }
}

void Segment::iteration()
{
    int neighbors;
    save_frame();
    for (int i = overlap_up; i < full_height() - overlap_down; i++)
    {
        for (int j = overlap_left; j < full_width() - overlap_right; j++)
        {
            neighbors = count_neighbors(j, i);
            if (prev_frame[i][j])
            {
                if (neighbors > 3 || neighbors < 2)
                    frame[i][j] = false;
            }
            else
            {
                if (neighbors == 3)
                    frame[i][j] = true;
            }
        }
    }
}
