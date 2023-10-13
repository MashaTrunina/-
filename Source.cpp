#include <iostream>
#include <fstream>
#include <vector>

#pragma pack(1) // предоставляет управление на уровне объявления данных
// расписываем структуру BMP файла 
struct BMPHeader {
    uint16_t signature; // 0x4d42
    uint32_t fileSize;  //размер файла
    uint16_t reserved1;  //0
    uint16_t reserved2;
    uint32_t dataOffset; // смещение данных
    uint32_t headerSize; // размер заголовка
    int32_t width;  // ширина
    int32_t height;  // высота
    uint16_t planes;  //длина
    uint16_t bitsPerPixel; // количество бит на пиксель
    uint32_t compression; // метод сжатия
    uint32_t dataSize;  // длина растрового массива 
    int32_t horizontalRes; // горизонтальное разрешение
    int32_t verticalRes;  //вертикальное разрешение
    uint32_t colors;  // число цветов изображения
    uint32_t importantColors; //число основных цветов
};

#pragma pack(pop)  // одноразовое использование части кода

void readBMP(const std::string& filename, BMPHeader& header, std::vector<unsigned char>& pixels) // чтение файла BMP, const= переменная является константой, 
{
    std::ifstream File(filename, std::ios::binary);  // открытие файла в конструкторе
    if (!File) 
    {
        std::cerr << "Error: Unable to open input file." << std::endl; // отображаем ошибки, если они имеются
        exit(1);
    }

    File.read(reinterpret_cast<char*>(&header), sizeof(header)); // читаем файл, преобразуем значение пустого указателя

    int width = header.width;
    int height = header.height;
    int rowSize = (width * header.bitsPerPixel / 8 + 3) & (~3); // проверка на кратность 4

    pixels.resize(rowSize * height); // массив с данными пикселей
    File.read(reinterpret_cast<char*>(pixels.data()), pixels.size());

    File.close();
}

void writeBMP(const std::string& filename, const BMPHeader& header, const std::vector<unsigned char>& pixels) // объявляем функцию
{
    std::ofstream outputFile(filename, std::ios::binary); //открываем файл, ios::bionory  будет использоваться для сохранения изображения  
    if (!outputFile) // условие верно, если файл открыт
    {
        std::cerr << "Error: Unable to open output file." << std::endl;
        exit(1);
    }

    outputFile.write(reinterpret_cast<const char*>(&header), sizeof(header)); // запись байтов из структуры header в файл
    outputFile.write(reinterpret_cast<const char*>(pixels.data()), pixels.size()); //  запись байтов из вектора pixels в файл

    outputFile.close(); // закрывает файл
}

  // поворачиваем картинку на 90 градусов 
 void rotateImage(std::vector<unsigned char>& pixels, int& width, int& height) {
    std::vector<unsigned char> rotatedPixels(pixels.size()); // создаем вектор для хранения пикселей повернутого изображения

    for (int x = 0; x < width; x++)  // предоставляем условия, для работы цикла
    {
        for (int y = 0; y < height; y++)
        {
            int srcOffset = (y * width + x) * 3;  // смещение в исходном векторе 
            int destOffset = ((width - x - 1) * height + y) * 3; // помещение пикселя в новое место после поворота
            rotatedPixels[destOffset] = pixels[srcOffset];  // копироование значения цветовых компонентов из исходного пикселя в новый вектор 
            rotatedPixels[destOffset + 1] = pixels[srcOffset + 1];
            rotatedPixels[destOffset + 2] = pixels[srcOffset + 2];
        }
    }

    pixels = rotatedPixels;
    std::swap(width, height); // Обновляем ширину и высоту после поворота
}

// использование фильтра Гаусса 
void GaussianFilter(std::vector<unsigned char>& pixels, int width, int height) {
    // Определение матрицы размером 5x5
    double kernel[5][5] = {
        {0.003, 0.013, 0.022, 0.013, 0.003},
        {0.013, 0.059, 0.097, 0.059, 0.013},
        {0.022, 0.097, 0.159, 0.097, 0.022},
        {0.013, 0.059, 0.097, 0.059, 0.013},
        {0.003, 0.013, 0.022, 0.013, 0.003}
    };

    int radius = 2; // радиус фильтра 
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

    pixels = filteredPixels; // Замена исходных пикселей отфильтрованными
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

    std::cout << "BMP файл перевернут в finish.bmp" << std::endl;

    GaussianFilter(pixels, header.width, header.height);
    writeBMP("finishG.bmp", header, pixels);

    std::cout << "Фильтр Гаусса готов" << std::endl;

    return 0;
}