#include <cmath>
#include "mcint.hpp"
#include "examplefuncs.hpp"

float 
fexp(float x)
{
        return std::exp(-x*x/2.0f);
}

float 
uni_pdf(float x)
{
        return 1.0f/4.0f;
}

float 
uni_inv_cdf(float x)
{
        return 2.0*x - 2.0;
}



#define GAMMA   0.1f

float 
log_pdf(float x)
{
        return GAMMA*std::exp(-GAMMA*(x - 1));
}

float 
log_inv_cdf(float y)
{
        return 1 - std::log(y)/GAMMA;
}


float 
fsin(const vec2& p)
{
        return sin(p.x);
}

float 
equi_pdf2(const vec2& z)
{
        return 1/(M_PI*M_PI);
}

vec2 
equi_inv_cdf2(const vec2& z)
{
        return vec2(M_PI/2*z.x, 2*M_PI*z.y);
}



float 
fsincos(const vec2& p)
{
        float v = sin(p.x)*cos(p.x)*cos(p.y);
        return v*v*sin(p.x);
}

float 
uni_pdf2(const vec2& z)
{
        return 1/(2*M_PI)*std::sin(z.x);
}

vec2 
uni_inv_cdf2(const vec2& z)
{
        return vec2(std::acos(z.x), 2*M_PI*z.y);
}

