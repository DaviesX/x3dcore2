#ifndef TEST_H
#define TEST_H

namespace test {

class if_test
{
public:
    if_test();
    virtual ~if_test();
    virtual void run() const = 0;

protected:
    float draw_rand() const;
};

} // namespace test

#endif // IF_TEST_H
