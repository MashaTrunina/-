#include <iostream>
#include <fstream>
#include <vector>
/* Следует разбивать программу на файлы. Во первых, просто ради
 * организации кода. Во вторых, чтобы была возможность проводить
 * раздельную компиляцию. Стоит помещать каждый класс\структуру 
 * в отдельный .h файл, а определения этих методов в .cpp. Тогда
 * при изменении реализации какой-то функции придется перекомпили-
 * ровать только этот .cpp файл. Если определение было в заголовке,
 * то придется перекомпилировать все файлы, в которые включен заголо-
 * вок (в крупном проекте это может сократить время компиляции на
 * несколько часов) */
#pragma pack(push, 1) // ������������� ���������� �� ������ ���������� ������
// ����������� ��������� BMP ����� 
/* А сколько из этих переменных ты на самом деле используешь?
 * А как сделать так, чтобы их стало меньше, но всё еще можно 
 * было бы считывать их одним чтением? */
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
/* Программа хорошо ложится на использование класса картинки. То есть
 * у нас мог бы быть класс BMP, который содержит все эти методы,
 * в приватных полях хранит указатель на пиксели и BMPHeader. При 
 * изменении картинки создает новый экземпляр картинки. Тогда мы бы 
 * имели более безопасную программу за счет инкапсуляции. */
void readBMP(const std::string& filename, BMPHeader& header, std::vector<unsigned char>& pixels) // ������ ����� BMP, const= ���������� �������� ����������, 
{
    std::ifstream File(filename, std::ios::binary);  // �������� ����� � ������������
    if (!File) /* Для проверки есть отдельный метод, Виктор Михайлович упоминал*/
    {
        std::cerr << "Error: Unable to open input file." << std::endl; // ���������� ������, ���� ��� �������
        exit(1);
    }

    File.read(reinterpret_cast<char*>(&header), sizeof(header)); // ������ ����, ����������� �������� ������� ���������
    /* Между заголовком и пикселями остались данные, которые ты не прочитала. 
     * Поэтому на моем компьютере твоя программа вовсе не работает. Эти данные
     * следовало считать, чтобы потом без изменений записать. А то потеряли*/
    int width = header.width;
    int height = header.height;
    /* Узнай обязательно, что это за операторы */
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
    /* Отлично, говорящий код */
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
    /* Получилась слишком глубокая вложенность. Подумай, как это
     * можно было бы разбить */
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

    readBMP("test3.bmp", header, pixels);

    rotateImage(pixels, header.width, header.height);

    header.fileSize = sizeof(header) + pixels.size();
    writeBMP("finish.bmp", header, pixels);

    std::cout << "BMP ���� ���������� � finish.bmp" << std::endl;

    GaussianFilter(pixels, header.width, header.height);
    writeBMP("finishG.bmp", header, pixels);

    std::cout << "������ ������ �����" << std::endl;

    return 0;
}