#ifndef IMAGE_EXPORT_HPP
#define IMAGE_EXPORT_HPP

#include <stdint.h>
#include <string>
#include <vector>

using namespace std;

#pragma pack(push, 1)

struct BMPFileHeader
{
    uint16_t file_type{0x4D42};
    uint32_t file_size{0};
    uint16_t reserved1{0};
    uint16_t reserved2{0};
    uint32_t offset_data{0};
};

struct BMPInfoHeader
{
    uint32_t size{0};
    int32_t width{0};
    int32_t height{0};
    uint16_t planes{1};
    uint16_t bit_count{24};
    uint32_t compression{0};
    uint32_t size_image{0};
    int32_t x_pixels_per_meter{0};
    int32_t y_pixels_per_meter{0};
    uint32_t colors_used{0};
    uint32_t colors_important{0};
};

struct BMPColorHeader
{
    uint32_t red_mask{0x00ff0000};
    uint32_t green_mask{0x0000ff00};
    uint32_t blue_mask{0x000000ff};
    uint32_t alpha_mask{0xff000000};
    uint32_t color_space_type{0x73524742};
    uint32_t unused[16] {0};
};

#pragma pack(pop)

class BMPExporter
{
private:
    BMPFileHeader file_header;
    BMPInfoHeader info_header;
    BMPColorHeader color_header;
    vector<uint8_t> image_data;
    uint32_t row_stride{0};
    int32_t scale;

    void write_headers(ofstream &output);
    void write_headers_and_data(ofstream &output);
    uint32_t make_stride_aligned(uint32_t align_stride);

public:
    BMPExporter(int32_t width, int32_t height, int32_t scale);
    void change_size_info(int32_t new_width, int32_t new_height);
    void write(string filename);
    void append(string filename);
    void pixel(uint32_t x, uint32_t y, uint8_t red, uint32_t green, uint32_t blue);
    void big_pixel(uint32_t x, uint32_t y, uint8_t red, uint32_t green, uint32_t blue);
};

#endif
