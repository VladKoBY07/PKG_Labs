#include "colormath.h"

#include <algorithm>
#include <cmath>

static double Value(double hue, double m1, double m2)
{
    while (hue < 0.0)
        hue += 360.0;
    while (hue >= 360.0)
        hue -= 360.0;

    if (hue < 60.0) {
        return m1 + (m2 - m1) * hue / 60.0;
    }
    if (hue < 180.0) {
        return m2;
    }
    if (hue < 240.0) {
        return m1 + (m2 - m1) * (240.0 - hue) / 60;
    }
    return m1;
}

Hls rgbToHls(double r, double g, double b)
{
    r /= 255.0;
    g /= 255.0;
    b /= 255.0;

    const double maximum = std::max({r, g, b});
    const double minimum = std::min({r, g, b});
    const double difference = maximum - minimum;

    Hls hls;
    hls.l = (maximum + minimum) / 2.0;

    if (difference == 0.0) {
        hls.h = 0.0;
        hls.s = 0.0;
        return hls;
    }

    if (hls.l <= 0.5) {
        hls.s = difference / (maximum + minimum);
    } else {
        hls.s = difference / (2.0 - maximum - minimum);
    }

    if (maximum == r) {
        hls.h = 60.0 * std::fmod((g - b) / difference, 6.0);
    } else if (maximum == g) {
        hls.h = 60.0 * ((b - r) / difference + 2.0);
    } else {
        hls.h = 60.0 * ((r - g) / difference + 4.0);
    }

    if (hls.h < 0.0) {
        hls.h += 360.0;
    }

    return hls;
}

void hlsToRgb(const Hls &hls, double &r, double &g, double &b)
{
    double h = hls.h;
    double l = hls.l;
    double s = hls.s;

    if (!std::isfinite(h) || !std::isfinite(l) || !std::isfinite(s)) {
        r = 0.0;
        g = 0.0;
        b = 0.0;
        return;
    }

    double m2;
    if (l < 0.5) {
        m2 = l * (1.0 + s);
    } else {
        m2 = l + s - l * s;
    }

    if (s < 1e-9) {
        if (h == -1.0 || std::isnan(h)) {
            r = 0.0;
            g = 0.0;
            b = 0.0;
            return;
        } else {
            r = l * 255.0;
            g = l * 255.0;
            b = l * 255.0;
            return;
        }
    } else {
        double m1 = 2.0 * l - m2;
        r = Value(h + 120.0, m1, m2) * 255.0;
        g = Value(h, m1, m2) * 255.0;
        b = Value(h - 120.0, m1, m2) * 255.0;
    }
}

// RGB -> XYZ

static double rgbxyzF(double x)
{
    if (x >= 0.04045) {
        return std::pow(((x + 0.055) / 1.055), 2.4);
    }
    return x / 12.92;
}

Xyz rgbToXyz(double r, double g, double b)
{
    if (!std::isfinite(r) || !std::isfinite(g) || !std::isfinite(b)) {
        return Xyz{0.0, 0.0, 0.0};
    }

    double Rn = rgbxyzF(r / 255) * 100;
    double Gn = rgbxyzF(g / 255) * 100;
    double Bn = rgbxyzF(b / 255) * 100;

    Xyz xyz;
    xyz.x = 0.412453 * Rn + 0.357580 * Gn + 0.180423 * Bn;
    xyz.y = 0.212671 * Rn + 0.715160 * Gn + 0.072169 * Bn;
    xyz.z = 0.019334 * Rn + 0.119193 * Gn + 0.950227 * Bn;

    return xyz;
}

// XYZ -> RGB

static double xyzrgbF(double x)
{
    if (!std::isfinite(x)) {
        return x;
    }

    if (x >= 0.0031308) {
        return 1.055 * std::pow(x, 1.0 / 2.4) - 0.055;
    }
    return 12.92 * x;
}

void xyzToRgb(const Xyz &xyz, double &r, double &g, double &b)
{
    double x = xyz.x;
    double y = xyz.y;
    double z = xyz.z;

    double Rn =  3.2406 * (x / 100) - 1.5372 * (y / 100) - 0.4986 * (z / 100);
    double Gn = -0.9689 * (x / 100) + 1.8758 * (y / 100) + 0.0415 * (z / 100);
    double Bn =  0.0557 * (x / 100) - 0.2040 * (y / 100) + 1.0570 * (z / 100);

    r = xyzrgbF(Rn) * 255;
    g = xyzrgbF(Gn) * 255;
    b = xyzrgbF(Bn) * 255;
}

// XYZ -> LAB

static double xyzlabF(double x)
{
    if (x >= 0.008856) {
        return std::pow(x, 1.0 / 3.0);
    }
    return 7.787 * x + 16.0 / 116.0;
}

Lab xyzToLab(const Xyz &xyz)
{
    Lab lab;

    double x = xyz.x;
    double y = xyz.y;
    double z = xyz.z;

    const double Xw = 95.047;
    const double Yw = 100.0;
    const double Zw = 108.883;

    lab.l = 116.0 * xyzlabF(y / Yw) - 16;
    lab.a = 500.0 * (xyzlabF(x / Xw) - xyzlabF(y / Yw));
    lab.b = 200.0 * (xyzlabF(y / Yw) - xyzlabF(z / Zw));

    return lab;
}

// LAB -> XYZ

static double labxyzF(double x)
{
    double xCube = x * x * x;
    if (xCube >= 0.008856) {
        return xCube;
    }
    return (x - 16.0 / 116.0) / 7.787;
}

Xyz labToXyz(const Lab &lab)
{
    Xyz xyz;

    double l = lab.l;
    double a = lab.a;
    double b = lab.b;

    const double Xw = 95.047;
    const double Yw = 100.0;
    const double Zw = 108.883;

    xyz.y = labxyzF((l + 16.0) / 116.0) * Yw;
    xyz.x = labxyzF(a / 500 + (l + 16.0) / 116.0) * Xw;
    xyz.z = labxyzF((l + 16.0) / 116.0 - b / 200.0) * Zw;

    return xyz;
}

bool isRgbInGamut(double r, double g, double b)
{
    constexpr double epsilon = 1e-9;

    return std::isfinite(r) && std::isfinite(g) && std::isfinite(b) &&
           r >= -epsilon && r <= 255.0 + epsilon &&
           g >= -epsilon && g <= 255.0 + epsilon &&
           b >= -epsilon && b <= 255.0 + epsilon;
}