#include <iostream>
#include <fstream>
#include <vector>

#pragma pack(1) // ������������� ���������� �� ������ ���������� ������
// ����������� ��������� BMP ����� 
struct BMPHeader {
    uint16_t signature; // 0x4d42
    uint32_t fileSize;  //������ �����
    uint16_t reserved1;  //0
    uint16_t reserved2;
    uint32_t dataOffset; // �������� ������
    uint32_t headerSize; // ������ ���������
    int32_t width;  // ������
    int32_t height;  // ������
    uint16_t planes;  //�����
    uint16_t bitsPerPixel; // ���������� ��� �� �������
    uint32_t compression; // ����� ������
    uint32_t dataSize;  // ����� ���������� ������� 
    int32_t horizontalRes; // �������������� ����������
    int32_t verticalRes;  //������������ ����������
    uint32_t colors;  // ����� ������ �����������
    uint32_t importantColors; //����� �������� ������
};

#pragma pack(pop)  // ����������� ������������� ����� ����

void readBMP(const std::string& filename, BMPHeader& header, std::vector<unsigned char>& pixels) // ������ ����� BMP, const= ���������� �������� ����������, 
{
    std::ifstream File(filename, std::ios::binary);  // �������� ����� � ������������
    if (!File) 
    {
        std::cerr << "Error: Unable to open input file." << std::endl; // ���������� ������, ���� ��� �������
        exit(1);
    }

    File.read(reinterpret_cast<char*>(&header), sizeof(header)); // ������ ����, ����������� �������� ������� ���������

    int width = header.width;
    int height = header.height;
    int rowSize = (width * header.bitsPerPixel / 8 + 3) & (~3); // �������� �� ��������� 4

    pixels.resize(rowSize * height); // ������ � ������� ��������
    File.read(reinterpret_cast<char*>(pixels.data()), pixels.size());

    File.close();
}

void writeBMP(const std::string& filename, const BMPHeader& header, const std::vector<unsigned char>& pixels) // ��������� �������
{
    std::ofstream outputFile(filename, std::ios::binary); //��������� ����, ios::bionory  ����� �������������� ��� ���������� �����������  
    if (!outputFile) // ������� �����, ���� ���� ������
    {
        std::cerr << "Error: Unable to open output file." << std::endl;
        exit(1);
    }

    outputFile.write(reinterpret_cast<const char*>(&header), sizeof(header)); // ������ ������ �� ��������� header � ����
    outputFile.write(reinterpret_cast<const char*>(pixels.data()), pixels.size()); //  ������ ������ �� ������� pixels � ����

    outputFile.close(); // ��������� ����
}

  // ������������ �������� �� 90 �������� 
 void rotateImage(std::vector<unsigned char>& pixels, int& width, int& height) {
    std::vector<unsigned char> rotatedPixels(pixels.size()); // ������� ������ ��� �������� �������� ����������� �����������

    for (int x = 0; x < width; x++)  // ������������� �������, ��� ������ �����
    {
        for (int y = 0; y < height; y++)
        {
            int srcOffset = (y * width + x) * 3;  // �������� � �������� ������� 
            int destOffset = ((width - x - 1) * height + y) * 3; // ��������� ������� � ����� ����� ����� ��������
            rotatedPixels[destOffset] = pixels[srcOffset];  // ������������ �������� �������� ����������� �� ��������� ������� � ����� ������ 
            rotatedPixels[destOffset + 1] = pixels[srcOffset + 1];
            rotatedPixels[destOffset + 2] = pixels[srcOffset + 2];
        }
    }

    pixels = rotatedPixels;
    std::swap(width, height); // ��������� ������ � ������ ����� ��������
}

// ������������� ������� ������ 
void GaussianFilter(std::vector<unsigned char>& pixels, int width, int height) {
    // ����������� ������� �������� 5x5
    double kernel[5][5] = {
        {0.003, 0.013, 0.022, 0.013, 0.003},
        {0.013, 0.059, 0.097, 0.059, 0.013},
        {0.022, 0.097, 0.159, 0.097, 0.022},
        {0.013, 0.059, 0.097, 0.059, 0.013},
        {0.003, 0.013, 0.022, 0.013, 0.003}
    };

    int radius = 2; // ������ ������� 
    std::vector<unsigned char> filteredPixels(pixels.size());

    for (int y = radius; y < height - radius; y++) {
        for (int x = radius; x < width - radius; x++) {
            double sumR = 0.0, sumG = 0.0, sumB = 0.0;

            for (int j = -radius; j <= radius; j++) {
                for (int i = -radius; i <= radius; i++) {
                    int pixelOffset = ((y + j) * width + (x + i)) * 3;
                    double weight = kernel[j + radius][i + radius];

                    sumR += pixels[pixelOffset] * weight;
                    sumG += pixels[pixelOffset + 1] * weight;
                    sumB += pixels[pixelOffset + 2] * weight;
                }
            }

            int offset = (y * width + x) * 3;
            filteredPixels[offset] = static_cast<unsigned char>(sumR);
            filteredPixels[offset + 1] = static_cast<unsigned char>(sumG);
            filteredPixels[offset + 2] = static_cast<unsigned char>(sumB);
        }
    }

    pixels = filteredPixels; // ������ �������� �������� ����������������
}


int main()
{
    setlocale(LC_ALL, "Russian");
    BMPHeader header;
    std::vector<unsigned char> pixels;

    readBMP("Head2.bmp", header, pixels);

    rotateImage(pixels, header.width, header.height);

    header.fileSize = sizeof(header) + pixels.size();
    writeBMP("finish.bmp", header, pixels);

    std::cout << "BMP ���� ���������� � finish.bmp" << std::endl;

    GaussianFilter(pixels, header.width, header.height);
    writeBMP("finishG.bmp", header, pixels);

    std::cout << "������ ������ �����" << std::endl;

    return 0;
}