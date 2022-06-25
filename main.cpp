#pragma once
#include <fstream>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#pragma pack(push, 1)
using namespace std;

struct  BMPFileHeader{
    uint16_t file_type{0x4D42};
    uint32_t file_size{0};
    uint16_t reserved1{0};
    uint16_t reserved2{0};
       uint32_t offset_data{0};
};

struct  BMPInfoHeader{
    uint32_t size{ 0 };
    int32_t width{ 0 };
    int32_t height{ 0 };
    uint16_t planes{ 1 };
    uint16_t bit_count{ 0 };
    uint32_t compression{ 0 };
    uint32_t size_image{ 0 };
    int32_t x_pixels_per_meter{ 0 };
    int32_t y_pixels_per_meter{ 0 };
    uint32_t colors_used{ 0 };
    uint32_t colors_important{ 0 };
};

struct BMPColorHeader {
    uint32_t red_mask{ 0x00ff0000 };
    uint32_t green_mask{ 0x0000ff00 };
    uint32_t blue_mask{ 0x000000ff };
    uint32_t alpha_mask{ 0xff000000 };
    uint32_t color_space_type{ 0x73524742 }; 
    uint32_t unused[16]{ 0 };
};
#pragma pack(pop) 
 
struct BMP {
    BMPFileHeader file_header;
    BMPInfoHeader bmp_info_header;
    BMPColorHeader bmp_color_header;
    std::vector<uint8_t> data;

    BMP(const char *fname) {
        read(fname);
    }

    void read(const char *fname) {
        std::ifstream inp{ fname, std::ios_base::binary };
        if (inp) {
            inp.read((char*)&file_header, sizeof(file_header));
            if(file_header.file_type != 0x4D42) {
                throw std::runtime_error("Error! Unrecognized file format.");
            }
            inp.read((char*)&bmp_info_header, sizeof(bmp_info_header));

            if(bmp_info_header.bit_count == 32) {

                if(bmp_info_header.size >= (sizeof(BMPInfoHeader) + sizeof(BMPColorHeader))) {
                    inp.read((char*)&bmp_color_header, sizeof(bmp_color_header));

                    check_color_header(bmp_color_header);
                } else {
                    std::cerr << "Error! The file \"" << fname << "\" does not seem to contain bit mask information\n";
                    throw std::runtime_error("Error! Unrecognized file format.");
                }
            }

            inp.seekg(file_header.offset_data, inp.beg);

            if(bmp_info_header.bit_count == 32) {
                bmp_info_header.size = sizeof(BMPInfoHeader) + sizeof(BMPColorHeader);
                file_header.offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + sizeof(BMPColorHeader);
            } else {
                bmp_info_header.size = sizeof(BMPInfoHeader);
                file_header.offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
            }
            file_header.file_size = file_header.offset_data;

            if (bmp_info_header.height < 0) {
                throw std::runtime_error("The program can treat only BMP images with the origin in the bottom left corner!");
            }

            data.resize(bmp_info_header.width * bmp_info_header.height * bmp_info_header.bit_count / 8);

            if (bmp_info_header.width % 4 == 0) {
                inp.read((char*)data.data(), data.size());
                file_header.file_size += static_cast<uint32_t>(data.size());
            }
            else {
                row_stride = bmp_info_header.width * bmp_info_header.bit_count / 8;
                uint32_t new_stride = make_stride_aligned(4);
                std::vector<uint8_t> padding_row(new_stride - row_stride);

                for (int y = 0; y < bmp_info_header.height; ++y) {
                    inp.read((char*)(data.data() + row_stride * y), row_stride);
                    inp.read((char*)padding_row.data(), padding_row.size());
                }
                file_header.file_size += static_cast<uint32_t>(data.size()) + bmp_info_header.height * static_cast<uint32_t>(padding_row.size());
            }
        }
        else {
            throw std::runtime_error("Unable to open the input image file.");
        }
    }

    BMP(int32_t width, int32_t height, bool has_alpha = true) {
        if (width <= 0 || height <= 0) {
            throw std::runtime_error("The image width and height must be positive numbers.");
        }

        bmp_info_header.width = width;
        bmp_info_header.height = height;
        if (has_alpha) {
            bmp_info_header.size = sizeof(BMPInfoHeader) + sizeof(BMPColorHeader);
            file_header.offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + sizeof(BMPColorHeader);

            bmp_info_header.bit_count = 32;
            bmp_info_header.compression = 3;
            row_stride = width * 4;
            data.resize(row_stride * height);
            file_header.file_size = file_header.offset_data + data.size();
        }
        else {
            bmp_info_header.size = sizeof(BMPInfoHeader);
            file_header.offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);

            bmp_info_header.bit_count = 24;
            bmp_info_header.compression = 0;
            row_stride = width * 3;
            data.resize(row_stride * height);

            uint32_t new_stride = make_stride_aligned(4);
            file_header.file_size = file_header.offset_data + static_cast<uint32_t>(data.size()) + bmp_info_header.height * (new_stride - row_stride);
        }
    }

    void write(const char *fname) {
        std::ofstream of{ fname, std::ios_base::binary };
        if (of) {
            if (bmp_info_header.bit_count == 32) {
                write_headers_and_data(of);
            }
            else if (bmp_info_header.bit_count == 24) {
                if (bmp_info_header.width % 4 == 0) {
                    write_headers_and_data(of);
                }
                else {
                    uint32_t new_stride = make_stride_aligned(4);
                    std::vector<uint8_t> padding_row(new_stride - row_stride);

                    write_headers(of);

                    for (int y = 0; y < bmp_info_header.height; ++y) {
                        of.write((const char*)(data.data() + row_stride * y), row_stride);
                        of.write((const char*)padding_row.data(), padding_row.size());
                    }
                }
            }
            else {
                throw std::runtime_error("The program can treat only 24 or 32 bits per pixel BMP files");
            }
        }
        else {
            throw std::runtime_error("Unable to open the output image file.");
        }
    }

	void read_pixel(uint32_t x0, uint32_t y0, uint8_t& B, uint8_t& G, uint8_t& R, uint8_t& A) {
		   if (x0 >= (uint32_t)bmp_info_header.width || y0 >= (uint32_t)bmp_info_header.height || x0 < 0 || y0 < 0) {
            throw std::runtime_error("The point is outside the image boundaries!");
        }
		
		uint32_t channels = bmp_info_header.bit_count / 8;
        B = data[channels * (y0 * bmp_info_header.width + x0) + 0];
        G = data[channels * (y0 * bmp_info_header.width + x0) + 1];
        R = data[channels * (y0 * bmp_info_header.width + x0) + 2];
        if (channels == 4) {
            A = data[channels * (y0 * bmp_info_header.width + x0) + 3];
        }
	}
    void set_pixel(uint32_t x0, uint32_t y0, uint8_t B, uint8_t G, uint8_t R, uint8_t A) {
        if (x0 >= (uint32_t)bmp_info_header.width || y0 >= (uint32_t)bmp_info_header.height || x0 < 0 || y0 < 0) {
            throw std::runtime_error("The point is outside the image boundaries!");
        }

        uint32_t channels = bmp_info_header.bit_count / 8;
        data[channels * (y0 * bmp_info_header.width + x0) + 0] = B;
        data[channels * (y0 * bmp_info_header.width + x0) + 1] = G;
        data[channels * (y0 * bmp_info_header.width + x0) + 2] = R;
        if (channels == 4) {
            data[channels * (y0 * bmp_info_header.width + x0) + 3] = A;
        }
    }



private:
    uint32_t row_stride{ 0 };

    void write_headers(std::ofstream &of) {
        of.write((const char*)&file_header, sizeof(file_header));
        of.write((const char*)&bmp_info_header, sizeof(bmp_info_header));
        if(bmp_info_header.bit_count == 32) {
            of.write((const char*)&bmp_color_header, sizeof(bmp_color_header));
        }
    }

    void write_headers_and_data(std::ofstream &of) {
        write_headers(of);
        of.write((const char*)data.data(), data.size());
    }

    uint32_t make_stride_aligned(uint32_t align_stride) {
        uint32_t new_stride = row_stride;
        while (new_stride % align_stride != 0) {
            new_stride++;
        }
        return new_stride;
    }

    void check_color_header(BMPColorHeader &bmp_color_header) {
        BMPColorHeader expected_color_header;
        if(expected_color_header.red_mask != bmp_color_header.red_mask ||
            expected_color_header.blue_mask != bmp_color_header.blue_mask ||
            expected_color_header.green_mask != bmp_color_header.green_mask ||
            expected_color_header.alpha_mask != bmp_color_header.alpha_mask) {
            throw std::runtime_error("Unexpected color mask format! The program expects the pixel data to be in the BGRA format");
        }
        if(expected_color_header.color_space_type != bmp_color_header.color_space_type) {
            throw std::runtime_error("Unexpected color space type! The program expects sRGB values");
        }
    }
};
template<typename T> void printElement(T t,const int& width){
    const char separator =' ';
        cout<< left << setw(width) << setfill(separator) << t;
    }

void readFileHeader(){
	BMP bmp("test.bmp");
	const int width = 10;
	printElement("File type",width);
	printElement(bmp.file_header.file_type,width);
	printElement("File size",width);
	printElement(bmp.file_header.file_size,width);
	printElement("Not used",width);
	printElement(bmp.file_header.reserved1,width);
	printElement("Not used",width);
	printElement(bmp.file_header.reserved2,width);
	printElement("Offset",width);
	printElement(bmp.file_header.offset_data,width);
}

void readBMPInfoHeader(){
	BMP bmp("test.bmp");
	const int width = 10;
	printElement("Size of Info Header",width);
	printElement(bmp.bmp_info_header.size,width);
	cout<<endl;
	printElement("Bitmap width",width);
	printElement(bmp.bmp_info_header.width,width);
	cout<<endl;
	printElement("Bitmap height",width);
	printElement(bmp.bmp_info_header.height,width);
	cout<<endl;
	printElement("numb. of planes",width);
	printElement(bmp.bmp_info_header.planes,width);
	cout<<endl;
	printElement("bits per pixel",width);
	printElement(bmp.bmp_info_header.bit_count,width);
	cout<<endl;
	printElement("compression",width);
	printElement(bmp.bmp_info_header.compression,width);
	cout<<endl;
	printElement("size",width);
	printElement(bmp.bmp_info_header.size_image,width);
	cout<<endl;
	printElement("X px/m",width);
	printElement(bmp.bmp_info_header.x_pixels_per_meter,width);
	cout<<endl;
	printElement("Y px/m",width);
	printElement(bmp.bmp_info_header.y_pixels_per_meter,width);
	cout<<endl;
	printElement("Colors",width);
	printElement(bmp.bmp_info_header.colors_used,width);
	cout<<endl;
	printElement("Colors impor.",width);
	printElement(bmp.bmp_info_header.colors_important,width);
}

void readBMPColorHeader(){
	BMP bmp("test.bmp");
	const int width = 10;
	printElement("red mask",width);
	printElement(bmp.bmp_color_header.red_mask,width);
	cout<<endl;	
	printElement("green mask",width);
	printElement(bmp.bmp_color_header.green_mask,width);
	cout<<endl;	
	printElement("blue mask",width);
	printElement(bmp.bmp_color_header.blue_mask,width);
	cout<<endl;
	printElement("alpha mask",width);
	printElement(bmp.bmp_color_header.alpha_mask,width);
	cout<<endl;
	printElement("color space",width);
	printElement(bmp.bmp_color_header.color_space_type,width);
	cout<<endl;
	printElement("unused",width);
	printElement(bmp.bmp_color_header.unused,width);
}
void negaitve(){
	uint8_t R,G,B,A;
	BMP bmp("test.bmp");
	BMP bmp2(bmp.bmp_info_header.width,bmp.bmp_info_header.height);
	for(uint32_t i=0; i<bmp.bmp_info_header.width; i++){
		for(uint32_t j=0; j<bmp.bmp_info_header.height;j++){
			bmp.read_pixel(i,j,B,G,R,A);
			bmp2.set_pixel(i,j,255-B,255-G,255-R,A);
		}
		
	}
	bmp2.write("testNegative.bmp");
}

int main(int argc, char** argv) {
    cout<<" File header"<<endl;
    readFileHeader();
    cout<<"\n\n\n";
    cout<<" BMP info header"<<endl;
    readBMPInfoHeader();
    cout<<"\n\n\n";
    cout<<" color header"<<endl;
    readBMPColorHeader();
    cout<<"\n\n\n";


    cout<<"preparing negative"<<endl;
    negative();
    return 0;
}
