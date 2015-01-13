#include <cstdio>
#include <algorithm>
#include <random>

typedef unsigned int uint;

struct Color {
    unsigned char r,g,b;
};

class Image
{
public:
    Image(uint width, uint height);
    ~Image();

    // non-copyable
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    Color& operator()(uint x, uint y) { return m_buffer[m_width*y + x]; }
    const Color& operator()(uint x, uint y) const { return m_buffer[m_width*y + x]; }

    uint m_width, m_height;
    Color* m_buffer;
};

Image::Image(uint width, uint height)
    : m_width(width)
    , m_height(height)
{
    m_buffer = new Color[width*height];
}
Image::~Image()
{
    delete[] m_buffer;
}
void writeImage(const Image& img, FILE* fp)
{
    fprintf(fp, "P3\n%u %u\n256\n", img.m_width, img.m_height);

    for (uint y=0; y < img.m_height; ++y) {
        for (uint x=0; x < img.m_width; ++x) {
            const Color& pixel = img(x,y);
            fprintf(fp, "%u %u %u ", pixel.r, pixel.g, pixel.b);
        }
        fprintf(fp, "\n");
    }
    fprintf(fp, "\n");
}



int main(int argc, char **argv)
{
    Image img(512, 512);

    std::default_random_engine rnd;
    std::uniform_real_distribution<double> point_chooser(-1.0, 1.0);
    std::uniform_int_distribution<int> func_chooser(0, 2);

    double x = point_chooser(rnd);
    double y = point_chooser(rnd);
    for (int it = 0; it < 100000; ++it) {
        int func_number = func_chooser(rnd);
        switch (func_number) {
        case 0:
            x = x/2;
            y = y/2;
            break;
        case 1:
            x = (x+1)/2;
            y = y/2;
            break;
        case 2:
            x = x/2;
            y = (y+1)/2;
            break;
        }
        if (it > 20) {
            img((x+1)/2*512, (y+1)/2*512) = Color{255,255,255};
        }
    }

    FILE* fp = fopen("test.ppm", "wb");
    if (!fp) {
        fprintf(stderr, "Couldn't open output file\n");
        return 1;
    }
    writeImage(img, fp);
    fclose(fp);
    return 0;
}
