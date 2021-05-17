#include <mpi.h>
#include <iostream>
#include <cstring>

#include "segment.hpp"

using namespace std;

void process(Segment segment, bool should_export, int frame_number, int rank, int process_count)
{
    BMPExporter exporter = BMPExporter(segment.full_frame_size, segment.height, 4);

    if (should_export)
        segment.iteration(exporter);
    else
        segment.iteration();

    bool *first_row = segment.frame[segment.overlap_up];
    bool *prev_row = rank == 0 ? NULL : segment.frame[0];
    bool *last_row = segment.frame[segment.full_height() - segment.overlap_down - 1];
    bool *next_row = rank == process_count - 1 ? NULL : segment.frame[segment.full_height() - 1];

    MPI_Status status;

    if (rank == process_count - 1)
    {
        if (should_export)
        {
            exporter.change_size_info(segment.full_frame_size, segment.full_frame_size);
            exporter.write("frames/frame" + to_string(frame_number) + ".bmp");
        }

	if (process_count > 1)
	{
            MPI_Send(first_row, segment.width, MPI_C_BOOL, rank - 1, 15, MPI_COMM_WORLD);
            MPI_Recv(prev_row, segment.width, MPI_C_BOOL, rank - 1, 15, MPI_COMM_WORLD, &status);
	}
    }
    else
    {
        if (process_count > 1)
        {
            MPI_Recv(next_row, segment.width, MPI_C_BOOL, rank + 1, 15, MPI_COMM_WORLD, &status);
            MPI_Send(last_row, segment.width, MPI_C_BOOL, rank + 1, 15, MPI_COMM_WORLD);
        }
        if (should_export)
            exporter.append("frames/frame" + to_string(frame_number) + ".bmp");

        if (rank > 0 && process_count > 1)
        {
            MPI_Send(first_row, segment.width, MPI_C_BOOL, rank - 1, 15, MPI_COMM_WORLD);
            MPI_Recv(prev_row, segment.width, MPI_C_BOOL, rank - 1, 15, MPI_COMM_WORLD, &status);
        }
    }
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

    int height = full_frame_size / process_count;
    int overlap_up = rank == 0 ? 0 : 1;
    int overlap_down = rank == process_count - 1 ? 0 : 1;
    int y = rank * height;

    Segment segment(pattern, full_frame_size, full_frame_size, height, overlap_up, overlap_down, 0, 0, 0, y, rank);

    time = MPI_Wtime();
    for (long long i = 0; i < iterations; i++)
        process(segment, should_export, i, rank, process_count);
    time = MPI_Wtime() - time;
    time /= iterations;
    cout << "proces " << rank << ": " << time << "s" << endl;

    segment.clean();
    MPI_Finalize();
    return 0;
}
