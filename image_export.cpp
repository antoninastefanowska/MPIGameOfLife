#include "image_export.hpp"

#include <fstream>

BMPExporter::BMPExporter(int32_t width, int32_t height, int32_t scale)
{
    if (width <= 0 || height <= 0)
        throw runtime_error("Niepoprawny rozmiar obrazu.");
    
    width *= scale;
    height *= scale;
    this->scale = scale;

    info_header.width = width;
    info_header.height = height;
    info_header.size = sizeof(BMPInfoHeader);
    file_header.offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
    row_stride = width * 3;
    image_data.resize(row_stride * height);

    uint32_t new_stride = make_stride_aligned(4);
    file_header.file_size = file_header.offset_data + image_data.size() + info_header.height * (new_stride - row_stride);
}

void BMPExporter::change_size_info(int32_t new_width, int32_t new_height)
{
    new_width *= scale;
    new_height *= scale;
    info_header.width = new_width;
    info_header.height = new_height;
}

void BMPExporter::append(string filename)
{
    ofstream output(filename, ios_base::binary | ios_base::app);
    if (output.is_open())
    {
        if (info_header.width % 4 == 0)
            output.write((const char*)image_data.data(), image_data.size());
        else
        {
            uint32_t new_stride = make_stride_aligned(4);
            vector<uint8_t> padding_row(new_stride - row_stride);
            for (int y = 0; y < info_header.height; y++)
            {
                output.write((const char*)(image_data.data() + row_stride * y), row_stride);
                output.write((const char*)padding_row.data(), padding_row.size());
            }
        }
        output.close();
    }
    else
        throw runtime_error("Nie można utworzyć pliku.");
}

void BMPExporter::write(string filename)
{
    ofstream output(filename, ios_base::binary);
    if (output.is_open())
    {
        if (info_header.width % 4 == 0)
            write_headers_and_data(output);
        else
        {
            uint32_t new_stride = make_stride_aligned(4);
            vector<uint8_t> padding_row(new_stride - row_stride);
            write_headers(output);
            for (int y = 0; y < info_header.height; y++)
            {
                output.write((const char*)(image_data.data() + row_stride * y), row_stride);
                output.write((const char*)padding_row.data(), padding_row.size());
            }
        }
        output.close();
    }
    else
        throw runtime_error("Nie można utworzyć pliku.");
}

void BMPExporter::pixel(uint32_t x, uint32_t y, uint8_t red, uint32_t green, uint32_t blue)
{
    if (x > info_header.width || y > info_header.height)
        throw runtime_error("Piksel poza granicami obrazu.");
    
    int index = 3 * ((info_header.height - y - 1) * info_header.width + x);
    image_data[index + 0] = blue;
    image_data[index + 1] = green;
    image_data[index + 2] = red;
}

void BMPExporter::big_pixel(uint32_t x, uint32_t y, uint8_t red, uint32_t green, uint32_t blue)
{
    for (int i = scale * y; i < scale * y + scale; i++)
        for (int j = scale * x; j < scale * x + scale; j++)
            pixel(j, i, red, green, blue);
}

void BMPExporter::write_headers(ofstream &output)
{
    output.write((const char*)&file_header, sizeof(file_header));
    output.write((const char*)&info_header, sizeof(info_header));
    if (info_header.bit_count == 32)
        output.write((const char*)&color_header, sizeof(color_header));
}

void BMPExporter::write_headers_and_data(ofstream &output)
{
    write_headers(output);
    output.write((const char*)image_data.data(), image_data.size());
}

uint32_t BMPExporter::make_stride_aligned(uint32_t align_stride)
{
    uint32_t new_stride = row_stride;
    while (new_stride % align_stride != 0)
        new_stride++;
    return new_stride;
}