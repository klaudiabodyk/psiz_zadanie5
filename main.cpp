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

struct BMPColorHeader {
    uint32_t red_mask{ 0x00ff0000 };
    uint32_t green_mask{ 0x0000ff00 };
    uint32_t blue_mask{ 0x000000ff };
    uint32_t alpha_mask{ 0xff000000 };
    uint32_t color_space_type{ 0x73524742 }; 
    uint32_t unused[16]{ 0 };
};

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
