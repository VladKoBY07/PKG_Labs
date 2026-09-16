#ifndef COLORMATH_H
#define COLORMATH_H

struct Hls
{
    double h;
    double l;
    double s;
};

struct Xyz
{
    double x;
    double y;
    double z;
};

struct Lab
{
    double l;
    double a;
    double b;
};

Hls rgbToHls(double r, double g, double b);
void hlsToRgb(const Hls &hls, double &r, double &g, double &b);
Xyz rgbToXyz(double r, double g, double b);
void xyzToRgb(const Xyz &xyz, double &r, double &g, double &b);
Lab xyzToLab(const Xyz &xyz);
Xyz labToXyz(const Lab &lab);
bool isRgbInGamut(double r, double g, double b);

#endif // COLORMATH_H