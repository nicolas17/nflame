/*
    nflame - fractal flame renderer
    Copyright (C) 2015 Nicolás Alvarez <nicolas.alvarez@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <cstdio>
#include <algorithm>
#include <random>
#include <array>

typedef unsigned int uint;

struct Color {
    unsigned char r,g,b;
};

template<typename C>
class Image
{
public:
    Image(uint width, uint height);
    ~Image();

    // non-copyable
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    /***/ C& pixel(uint x, uint y) /***/ { return m_buffer[m_width*y + x]; }
    const C& pixel(uint x, uint y) const { return m_buffer[m_width*y + x]; }

    uint m_width, m_height;
    C* m_buffer;
};

template<typename C>
Image<C>::Image(uint width, uint height)
    : m_width(width)
    , m_height(height)
{
    m_buffer = new C[width*height];
}
template<typename C>
Image<C>::~Image()
{
    delete[] m_buffer;
}

void writeImage(const Image<Color>& img, FILE* fp)
{
    fprintf(fp, "P3\n%u %u\n256\n", img.m_width, img.m_height);

    for (uint y=0; y < img.m_height; ++y) {
        for (uint x=0; x < img.m_width; ++x) {
            const Color& pixel = img.pixel(x,y);
            fprintf(fp, "%u %u %u ", pixel.r, pixel.g, pixel.b);
        }
        fprintf(fp, "\n");
    }
    fprintf(fp, "\n");
}

class Point {
public:
    double x,y;
    Point& operator+=(Point p) {
        x += p.x;
        y += p.y;
        return *this;
    }
    Point& operator*=(double f) {
        x *= f;
        y *= f;
        return *this;
    }
};
Point operator*(Point p, double f) {
    return {p.x*f, p.y*f};
}

inline Point sinusoidal(double x, double y) {
    return {sin(x), sin(y)};
}
inline Point spherical(double x, double y) {
    const double rsq = x*x+y*y;
    return {x/rsq, y/rsq};
}
inline Point affine_transform(Point p, double a, double b, double c, double d, double e, double f) {
    return {a*p.x + b*p.y + c, d*p.x + e*p.y + f};
}
struct Xform
{
    /** affine transform coefficients */
    double a,d,b,e,c,f;

    double w_sinusoidal, w_spherical;
};

Point apply_transform(Point p, const Xform& xform) {
    Point rp{0.0, 0.0};
    p = affine_transform(p, xform.a, xform.b, xform.c, xform.d, xform.e, xform.f);

    rp += sinusoidal(p.x, p.y) * xform.w_sinusoidal;
    rp += spherical(p.x, p.y) * xform.w_spherical;
    return rp;
}

int main(int argc, char **argv)
{
    Image<Color> img(512, 512);
    Image<double> histogram(512, 512);

    std::default_random_engine rnd;
    std::uniform_real_distribution<double> real_dist(-1.0, 1.0);
    std::uniform_int_distribution<int> xform_dist(0, 4);

    std::array<Xform, 5> xforms = {{
        { -0.233501, 0.351438, 0.0400411, -0.0749041, 0.568229, -0.272239, 1, 0},
        { -0.890786, 0.81055, -0.94619, 0.943775, 0.72818, -0.854725, 0.58651, 0.41349},
        { -0.897478, 0.432736, -0.297757, -0.879828, 0.659483, 0.408992, 0.185739, 0.814261},
        { 0.0951644, -0.642964, 0.0715653, -0.336607, 0.0847969, -0.333947, 0.473191, 0.526809},
        { 0.149391, 0.0891495, 0.662109, -0.191126, -0.501858, -0.656649, 0.501891, 0.498109}
    }};

    Point p{real_dist(rnd), real_dist(rnd)};
    for (int it = 0; it < 100000000; ++it) {
        int nxform = xform_dist(rnd);
        p = apply_transform(p, xforms[nxform]);

        if (it > 20) {
            if (p.x > -1 && p.x < 1 && p.y > -1 && p.y < 1) {
                histogram.pixel((p.x+1)/2*img.m_width, (p.y+1)/2*img.m_height) += 1;
            }
        }
    }

    double max_sample = *std::max_element(&histogram.m_buffer[0], &histogram.m_buffer[histogram.m_width * histogram.m_height]);
    for (uint y=0; y < img.m_height; ++y) {
        for (uint x=0; x < img.m_width; ++x) {
            int p = log(histogram.pixel(x,y))*(256./log(max_sample));
            img.pixel(x,y) = {p,p,p};
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
