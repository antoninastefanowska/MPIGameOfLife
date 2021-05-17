#include <mpi.h>
#include <cmath>
#include <cstring>

#include "segment.hpp"

void export_segment(Segment segment, BMPExporter &exporter, bool *children_data, bool new_file, int frame_number, int rank)
{
    for (int i = 0; i < segment.height; i++)
    {
        for (int j = 0; j < segment.x; j++)
        {
            if (children_data[i * segment.x + j])
                exporter.big_pixel(j, i, 255, 255, 255);
            else
                exporter.big_pixel(j, i, 0, 0, 0);
        }
        for (int j = 0; j < segment.width; j++)
        {
            if (segment.frame[i + segment.overlap_up][j + segment.overlap_left])
                exporter.big_pixel(j + segment.x, i, 255, 255, 255);
            else
                exporter.big_pixel(j + segment.x, i, 0, 0, 0);
        }
    }

    if (new_file)
    {
        exporter.change_size_info(segment.full_frame_size, segment.full_frame_size);
        exporter.write("frames/frame" + to_string(frame_number) + ".bmp");
    }
    else
        exporter.append("frames/frame" + to_string(frame_number) + ".bmp");
}

void process(Segment segment, bool should_export, int frame_number, int block_x, int block_y, int k, int base_size)
{
    BMPExporter exporter = BMPExporter(segment.full_frame_size, segment.height, 4);

    bool *first_row = segment.frame[segment.overlap_up];
    bool *prev_row = block_y == 0 ? NULL : segment.frame[0];
    bool *last_row = segment.frame[segment.full_height() - segment.overlap_down - 1];
    bool *next_row = block_y == k - 1 ? NULL : segment.frame[segment.full_height() - 1];

    bool *first_col = block_x == 0 ? NULL : new bool[segment.height];
    bool *next_col = block_x == k - 1 ? NULL : new bool[segment.height];

    MPI_Status status;
    int rank = block_y * k + block_x;
    int children_data_size, current_data_size;
    bool *children_data, *current_data;

    if (block_x > 0)
    {
        children_data_size = segment.x * segment.height;
        current_data_size = children_data_size + segment.width * segment.height;
        children_data = new bool[children_data_size];

        MPI_Recv(children_data, children_data_size, MPI_C_BOOL, rank - 1, 15, MPI_COMM_WORLD, &status);

        for (int i = 0; i < segment.height; i++)
        {
            segment.frame[i + segment.overlap_up][0] = children_data[(i + 1) * segment.x - 1];
            first_col[i] = segment.frame[i + segment.overlap_up][segment.overlap_left];
        }
        MPI_Send(first_col, segment.height, MPI_C_BOOL, rank - 1, 16, MPI_COMM_WORLD);
        delete [] first_col;
    }
    else
    {
        children_data_size = 0;
        current_data_size = base_size * segment.height;
        children_data = new bool[current_data_size];
    }

    if (block_x < k - 1)
    {
        current_data = new bool[current_data_size];
        for (int i = 0; i < segment.height; i++)
        {
            for (int j = 0; j < segment.x; j++)
                current_data[i * (segment.x + base_size) + j] = children_data[i * segment.x + j];
            for (int j = 0; j < segment.width; j++)
                current_data[i * (segment.x + base_size) + j + segment.x] = segment.frame[i + segment.overlap_up][j + segment.overlap_left];
        }

        MPI_Send(current_data, current_data_size, MPI_C_BOOL, rank + 1, 15, MPI_COMM_WORLD);
        MPI_Recv(next_col, segment.height, MPI_C_BOOL, rank + 1, 16, MPI_COMM_WORLD, &status);

        for (int i = 0; i < segment.height; i++)
            segment.frame[i + segment.overlap_up][segment.full_width() - 1] = next_col[i];

        delete [] children_data;
        delete [] current_data;
        delete [] next_col;
    }

    if (block_y == k - 1)
    {
        int target_rank = (block_y - 1) * k + block_x;
        if (block_x == k - 1)
        {
            if (should_export)
                export_segment(segment, exporter, children_data, true, frame_number, rank);
            delete [] children_data;
        }
        MPI_Send(first_row, segment.full_width(), MPI_C_BOOL, target_rank, 17, MPI_COMM_WORLD);
        MPI_Recv(prev_row, segment.full_width(), MPI_C_BOOL, target_rank, 17, MPI_COMM_WORLD, &status);
    }
    else
    {
        int target_rank = (block_y + 1) * k + block_x;
        MPI_Recv(next_row, segment.full_width(), MPI_C_BOOL, target_rank, 17, MPI_COMM_WORLD, &status);
        MPI_Send(last_row, segment.full_width(), MPI_C_BOOL, target_rank, 17, MPI_COMM_WORLD);

        if (block_x == k - 1)
        {
            if (should_export)
                export_segment(segment, exporter, children_data, false, frame_number, rank);
            delete [] children_data;
        }
        if (block_y > 0)
        {
            int target_rank = (block_y - 1) * k + block_x;
            MPI_Send(first_row, segment.full_width(), MPI_C_BOOL, target_rank, 17, MPI_COMM_WORLD);
            MPI_Recv(prev_row, segment.full_width(), MPI_C_BOOL, target_rank, 17, MPI_COMM_WORLD, &status);
        }
    }
    segment.iteration();
}

int main(int argc, char *argv[])
{
    int full_frame_size = atoi(argv[1]), pattern_number = atoi(argv[3]), process_count, rank;
    long long iterations = atoll(argv[2]);
    long double time;
    bool should_export = argc > 4 && strcmp(argv[4], "-e") == 0;
    PatternType pattern = static_cast<PatternType>(pattern_number);

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &process_count);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int k = (int)sqrt(process_count);
    int base_size = ceil((float)full_frame_size / k);
    int block_x = rank % k, block_y = rank / k;

    int overlap_up = block_y == 0 ? 0 : 1;
    int overlap_down = block_y == k - 1 ? 0 : 1;
    int overlap_left = block_x == 0 ? 0 : 1;
    int overlap_right = block_x == k - 1 ? 0 : 1;

    int x = block_x * base_size, y = block_y * base_size;
    int width = base_size, height = base_size;

    if (base_size * k > full_frame_size)
    {
        if (block_x == k - 1)
            width = base_size * k - full_frame_size + 1;
        if (block_y == k - 1)
            height = base_size * k - full_frame_size + 1;
    }

    Segment segment(pattern, full_frame_size, width, height, overlap_up, overlap_down, overlap_left, overlap_right, x, y, rank);

    time = MPI_Wtime();
    for (int i = 0; i < iterations; i++)
        process(segment, should_export, i, block_x, block_y, k, base_size);
    time = MPI_Wtime() - time;
    time /= iterations;
    cout<< "proces " << rank << ": " << time << "s" << endl;

    segment.clean();
    MPI_Finalize();
    return 0;
}
