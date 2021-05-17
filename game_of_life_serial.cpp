#include <mpi.h>
#include <cstring>

#include "segment.hpp"

int main(int argc, char *argv[])
{
    int full_frame_size = atoi(argv[1]), pattern_number = atoi(argv[3]);
    long long iterations = atoll(argv[2]);
    long double time;
    bool should_export = argc > 4 && strcmp(argv[4], "-e") == 0;
    PatternType pattern = static_cast<PatternType>(pattern_number);
    BMPExporter exporter(full_frame_size, full_frame_size, 4);
    Segment segment(pattern, full_frame_size, full_frame_size, full_frame_size, 0, 0, 0, 0, 0, 0, 0);

    MPI_Init(&argc, &argv);

    time = MPI_Wtime();
    for (long long i = 0; i < iterations; i++)
    {
        if (should_export)
        {
            segment.iteration(exporter);
            exporter.write("frames/frame" + to_string(i) + ".bmp");
        }
        else
            segment.iteration();
    }
    time = MPI_Wtime() - time;
    time /= iterations;
    cout << "szeregowo: " << time << "s" << endl;

    segment.clean();
    MPI_Finalize();
    return 0;
}
