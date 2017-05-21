#include <iostream>

#include "../src/tensor.h"
#include "test_tensor.h"

test::test_tensor::test_tensor()
{
}

void
test::test_tensor::run() const
{
        // Test vector.
        e8util::vec3 a;
        e8util::vec3 b(0.0f);
        e8util::vec3 c({0.0f, 0.0f, 0.0f});
        assert(a == b && b == c);

        for (unsigned i = 0; i < 1000; i ++) {
                float n = draw_rand();
                float m = draw_rand();
                float o = draw_rand();

                e8util::vec3 x({n, m, o});
                e8util::vec3 y({-n, -m, -o});
                assert(x + y == e8util::vec3(0.0f));
                assert(x - y == 2.0f*e8util::vec3({n, m, o}));
                assert(x * y == e8util::vec3({-n*n, -m*m, -o*o}));
                assert(-x * -y == e8util::vec3({-n*n, -m*m, -o*o}));
                assert(!(x * y != e8util::vec3({-n*n, -m*m, -o*o})));

                a = {draw_rand(), draw_rand(), draw_rand()};
                b = {draw_rand(), draw_rand(), draw_rand()};
                c = {draw_rand(), draw_rand(), draw_rand()};

                assert(a.outer(b.outer(c)) + c.outer(a.outer(b)) + b.outer(c.outer(a)) == 0);
                assert(e8util::equals((a + b).inner(c), a.inner(c) + b.inner(c)));
                assert(e8util::equals(a.inner(b.outer(c)), b.inner(c.outer(a))));
        }

        // Test matrix.
        e8util::mat33           m = {1, 3, 2,
                                     2, 2, 1,
                                     3, 1, 2};
        e8util::mat31        m2 = {1,
                                   4,
                                   7};
        e8util::mat31 m3 = m*m2;
        assert((m3 == e8util::mat31({30, 18, 20})));

        e8util::mat33 l, u;
        m.lu_decompose(l, u);
        e8util::mat33 lu = l*u;
        assert(lu == m);

        e8util::vec3 x = m.solve(e8util::vec3({1,4,7}));
        assert(x == e8util::vec3({3.6250, -4.5000, 2.1250}));

        e8util::mat33   k = {-3, -1, 3,
                              2,  0,-4,
                             -5, -2, 1};

        assert(k.adjugate() == e8util::mat33({-8, -5, 4,
                                               18, 12, -6,
                                               -4, -1, 2}));

        e8util::mat33   k2 = {-2, -1, 2,
                               2, 1,  0,
                              -3, 3, -1};
        assert(e8util::equals(k2.det(), 18));

        e8util::mat33 m_p = m^(-1);
        assert(m_p*m == m*m_p && m_p*m == 1.0f);

        x = m.ls_solve(e8util::vec3({1,4,7}));
        assert(x == e8util::vec3({3.6250, -4.5000, 2.1250}));

#define DIMENSION 10
        for (unsigned i = 0; i < 100; i ++) {
                e8util::mat<DIMENSION, DIMENSION, double> m;
                for (unsigned j = 0; j < DIMENSION; j ++) {
                        for (unsigned i = 0; i < DIMENSION; i ++) {
                                m(i,j) = draw_rand()*100;
                        }
                }
                assert(m + m == 2.0*m);
                assert((m - m == e8util::mat<DIMENSION, DIMENSION, double>()));
                assert(-m + m == m - m);

                e8util::mat<DIMENSION, DIMENSION, double> n = m^(-1);
                e8util::mat<DIMENSION, DIMENSION, double> I = m*n;
                assert(I == 1.0f);
        }
}
