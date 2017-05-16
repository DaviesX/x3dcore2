#include <iostream>
#include <random>
#include <cmath>
#include "mcint.hpp"


std::ostream&
operator<<(std::ostream& os, const mcint_result& r)
{
        os << r.F << "\t" << r.ss;
        return os;
}


mcint_result
mcint_1d(oned_func_t f, oned_func_t pdf, oned_inv_func_t inv_cdf, int seed, unsigned N)
{
        std::uniform_real_distribution<float> dist(0, 1);
        std::mt19937 gen(seed);

        float sum = 0;
        float sum2 = 0;
        for (unsigned i = 0; i < N; i++) {
                float x = inv_cdf(dist(gen));
                float p = pdf(x);
                float fx = f(x);
                float Y = fx/p;
                sum += Y;
                sum2 += Y*Y;
        }
        mcint_result r;
        r.F = sum/N;
        float s = std::sqrt(1.0/(N-1)*sum2 - N/static_cast<float>(N-1)*r.F*r.F);
        r.ss = 2*s/std::sqrt(N);
        return r;
}

mcint_result
mcint_2d(twod_func_t f, twod_func_t pdf, twod_inv_func_t inv_cdf, int seed, unsigned N)
{
        std::uniform_real_distribution<float> dist(0, 1);
        std::mt19937 gen(seed);

        float sum = 0;
        float sum2 = 0;
        for (unsigned i = 0; i < N; i++) {
                vec2 z(dist(gen), dist(gen));
                const vec2& X = inv_cdf(z);
                float p = pdf(X);
                float fx = f(X);
                float Y = fx/p;
                sum += Y;
                sum2 += Y*Y;
        }
        mcint_result r;
        r.F = sum/N;
        float s = std::sqrt(1.0/(N-1)*sum2 - N/static_cast<float>(N-1)*r.F*r.F);
        r.ss = 2*s/std::sqrt(N);
        return r;
}

bool is_visible(const vec3& source, float radius, const vec3& o, const vec3& d)
{
        vec3 s(o.x - source.x, o.y - source.y, o.z - source.z);
        float a = d.x*d.x + d.y*d.y + d.z*d.z;
        float b = 2*(s.x*d.x + s.y*d.y + s.z*d.z);
        float c = s.x*s.x + s.y*s.y + s.z*s.z - radius*radius;
        float delta = b*b - 4*a*c;
        if (delta < 0)
                return false;
        return true;
}

mcint_result
mcint_flux(const vec3& unit_square, const vec3& source, float radius, float intensity, int seed, unsigned N)
{
        std::uniform_real_distribution<float> dist(0, 1);
        std::mt19937 gen(seed);

        const float plane_w = 1;
        const float plane_h = 1;

        float sum = 0;
        float sum2 = 0;
        for (unsigned i = 0; i < N; i++) {
                float u = dist(gen);
                float v = dist(gen);

                float z = 2*u - 1;
                float x = std::sqrt(1 - z*z)*std::cos(2*M_PI*v);
                float y = std::sqrt(1 - z*z)*std::sin(2*M_PI*v);

                vec3 n(-x,-y,-z);
                vec3 target(source.x + x*radius, source.y + y*radius, source.z + z*radius);
                vec3 p0(unit_square.x + dist(gen) - plane_w/2.0f, unit_square.y + dist(gen) - plane_h/2.0f, unit_square.z);
                vec3 r(target.x - p0.x, target.y - p0.y, target.z - p0.z);
                float d2 = r.x*r.x + r.y*r.y + r.z*r.z;
                float d = std::sqrt(d2);

                r.x /= d;
                r.y /= d;
                r.z /= d;

                float cos_the = r.z;
                float cos_phi = n.x*r.x + n.y*r.y + n.z*r.z;

                if (cos_phi < 0)
                        continue;

                float f = intensity*cos_the*cos_phi/d2;
                float p = 1.0f/(4*M_PI*radius*radius)*1.0f/(1.0f*1.0f);
                float Y = f/p;
                sum += Y;
                sum2 += Y*Y;
        }
        mcint_result r;
        r.F = sum/N;
        float s = std::sqrt(1.0/(N-1)*sum2 - N/static_cast<float>(N-1)*r.F*r.F);
        r.ss = 2*s/std::sqrt(N);
        return r;
}

mcint_result
mcint_flux0(const vec3& unit_square, const vec3& source, float radius, float intensity, int seed, unsigned N)
{
        std::uniform_real_distribution<float> dist(0, 1);
        std::mt19937 gen(seed);

        const float plane_w = 1;
        const float plane_h = 1;

        float sum = 0;
        float sum2 = 0;
        for (unsigned i = 0; i < N; i++) {
                vec3 x(unit_square.x + dist(gen) - plane_w/2.0f, unit_square.y + dist(gen) - plane_h/2.0f, unit_square.z);
                vec2 u(std::acos(dist(gen)), 2*M_PI*dist(gen));
                vec3 d(std::cos(u.x)*std::cos(u.y), std::cos(u.x)*std::sin(u.y), std::sin(u.x));

                if (!is_visible(source, radius, x, d)) {
                        continue;
                }
                float f = intensity*std::sin(u.x);
                float p = 1/(2*M_PI);
                float y = f/p*1.0f/(1.0f*1.0f);
                sum += y;
                sum2 += y*y;
        }
        mcint_result r;
        r.F = sum/N;
        float s = std::sqrt(1.0/(N-1)*sum2 - N/static_cast<float>(N-1)*r.F*r.F);
        r.ss = 2*s/std::sqrt(N);
        return r;
}

