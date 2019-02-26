#ifndef TESTUNIDIRECTRENDERER_H
#define TESTUNIDIRECTRENDERER_H

#include "test.h"

namespace test
{

class test_unidirect_renderer: public if_test
{
public:
        test_unidirect_renderer();
        ~test_unidirect_renderer() override;

        void    run() const override;
};

}

#endif // TESTUNIDIRECTRENDERER_H
