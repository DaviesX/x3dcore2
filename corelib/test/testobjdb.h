#ifndef TESTOBJDB_H
#define TESTOBJDB_H


#include "test.h"

namespace test
{

class test_objdb: public if_test
{
public:
        test_objdb();
        ~test_objdb();

        void run() const override;
};

}

#endif // TESTOBJDB_H
