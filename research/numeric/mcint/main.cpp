#include <iostream>
#include "mcint.hpp"
#include "examplefuncs.hpp"


int main()
{
        std::cout << "Test 1" << std::endl;
        for (int i = 0; i < 10; i ++) {
                mcint_result r = mcint_1d(fexp, uni_pdf, uni_inv_cdf, i*1237, 100000);
                std::cout << r << std::endl;
        }

        std::cout << "Test 2" << std::endl;
        for (int i = 0; i < 10; i ++) {
                mcint_result r = mcint_1d(fexp, log_pdf, log_inv_cdf, i*1237, 100000);
                std::cout << r << std::endl;
        }

        std::cout << "Test 3" << std::endl;
        for (int i = 0; i < 10; i ++) {
                mcint_result r = mcint_2d(fsin, equi_pdf2, equi_inv_cdf2, i*1237, 100000);
                std::cout << r << std::endl;
        }

        std::cout << "Test 4" << std::endl;
        for (int i = 0; i < 10; i ++) {
                mcint_result r = mcint_2d(fsincos, uni_pdf2, uni_inv_cdf2, i*1237, 100000);
                std::cout << r << std::endl;
        }

        std::cout << "Test 5" << std::endl;
        for (int i = 0; i < 10; i ++) {
                mcint_result r = mcint_flux0(vec3(0,0,0), vec3(1,1,5), 1, 100, i*1237, 20000);
                std::cout << r << std::endl;
        }

        std::cout << "Test 6" << std::endl;
        for (int i = 0; i < 10; i ++) {
                mcint_result r = mcint_flux(vec3(0,0,0), vec3(1,1,5), 1, 100, i*1237, 1000000);
                std::cout << r << std::endl;
        }
        return 0;
}
