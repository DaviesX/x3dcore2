#ifndef MCINT_HPP
#define MCINT_HPP

#include <ostream>


struct vec2
{
        float x;
        float y;

        vec2(float x, float y):
                x(x), y(y)
        {
        }
};

struct vec3
{
        float x;
        float y;
        float z;

        vec3(float x, float y, float z):
                x(x), y(y), z(z)
        {
        }
};

typedef float (*oned_func_t) (float x);
typedef float (*oned_inv_func_t) (float y);

typedef float (*twod_func_t) (const vec2& x);
typedef vec2 (*twod_inv_func_t) (const vec2& z);

struct mcint_result
{
        float F;
        float ss;
};

std::ostream& operator<<(std::ostream& os, const mcint_result& r);

mcint_result    mcint_1d(oned_func_t f, oned_func_t pdf, oned_inv_func_t inv_cdf, int seed, unsigned N);
mcint_result    mcint_2d(twod_func_t f, twod_func_t pdf, twod_inv_func_t inv_cdf, int seed, unsigned N);

mcint_result    mcint_flux(const vec3& unit_square, const vec3& source, float radius, float intensity, int seed, unsigned N);
mcint_result    mcint_flux0(const vec3& unit_square, const vec3& source, float radius, float intensity, int seed, unsigned N);

#endif
