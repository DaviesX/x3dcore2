#ifndef EXAMPLEFUNCS_HPP
#define EXAMPLEFUNCS_HPP

float   fexp(float x);
float   uni_pdf(float x);
float   uni_inv_cdf(float y);

float   log_pdf(float x);
float   log_inv_cdf(float y);

float   fsin(const vec2& p);
float   equi_pdf2(const vec2& z);
vec2    equi_inv_cdf2(const vec2& z);

float   fsincos(const vec2& p);
float   uni_pdf2(const vec2& z);
vec2    uni_inv_cdf2(const vec2& z);


#endif
