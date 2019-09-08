#ifndef TESTOBJDB_H
#define TESTOBJDB_H

#include "test.h"

namespace test {

class test_objdb : public if_test
{
public:
    test_objdb();
    ~test_objdb() override;

    void run() const override;
};

} // namespace test

#endif // TESTOBJDB_H
