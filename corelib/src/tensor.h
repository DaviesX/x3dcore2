#ifndef TENSOR_H
#define TENSOR_H

#include <cassert>
#include <cmath>
#include <initializer_list>
#include <ostream>
#include <random>
#include <string>
#include <vector>

namespace e8util {

#define T_CMP_EPSILON 1e-5f

inline bool equals(float a, float b) { return std::abs(a - b) <= T_CMP_EPSILON; }

inline bool equals(double a, double b) {
    return std::abs(a - b) <= static_cast<double>(T_CMP_EPSILON);
}

template <typename T> bool equals(std::vector<T> const &a, std::vector<T> const &b) {
    assert(a.size() == b.size());
    for (unsigned i = 0; i < a.size(); i++) {
        if (!equals(a[i], b[i])) {
            return false;
        }
    }
    return true;
}

template <unsigned N, typename T = float> class vec {
  public:
    vec(std::initializer_list<T> const &v);
    vec(T const *v);
    vec(T const &v);
    vec();

    vec operator+(vec const &rhs) const;
    vec operator-(vec const &rhs) const;
    vec operator-() const;
    vec operator*(T k) const;
    vec operator/(T k) const;
    T operator()(unsigned i) const;
    T &operator()(unsigned i);
    bool operator==(vec const &rhs) const;
    bool operator!=(vec const &rhs) const;

    vec operator*(vec const &rhs) const; // modulation, not cross product.
    vec operator/(vec const &rhs) const;

    vec &operator+=(vec const &rhs);
    vec &operator-=(vec const &rhs);
    vec &operator*=(vec const &rhs); // modulation, not cross product.

    T inner(vec const &rhs) const;
    vec outer(vec const &rhs) const;

    T norm() const;
    T norm2() const;
    vec normalize() const;

    vec<N + 1, T> homo(T p) const;
    vec<N - 1, T> cart() const;
    vec<N - 1, T> trunc() const;

    vec at_least(T const &val) const;
    vec at_most(T const &val) const;

  private:
    T e[N];
};

typedef vec<2> vec2;
typedef vec<3> vec3;
typedef vec<4> vec4;

template <unsigned N, typename T> vec<N, T>::vec(std::initializer_list<T> const &v) {
    auto it = v.begin();
    for (unsigned i = 0; i < N; i++) {
        (*this)(i) = *it;
        ++it;
    }
}

template <unsigned N, typename T> vec<N, T>::vec(T const *v) {
    for (unsigned i = 0; i < N; i++)
        (*this)(i) = v[i];
}

template <unsigned N, typename T> vec<N, T>::vec(T const &v) {
    for (unsigned i = 0; i < N; i++)
        (*this)(i) = v;
}

template <unsigned N, typename T> vec<N, T>::vec() {
    for (unsigned i = 0; i < N; i++)
        (*this)(i) = 0;
}

template <unsigned N, typename T> vec<N, T> vec<N, T>::operator+(vec const &rhs) const {
    vec<N, T> v;
    for (unsigned i = 0; i < N; i++)
        v(i) = (*this)(i) + rhs(i);
    return v;
}

template <unsigned N, typename T> T &vec<N, T>::operator()(unsigned i) { return e[i]; }

template <unsigned N, typename T> T vec<N, T>::operator()(unsigned i) const { return e[i]; }

template <unsigned N, typename T> bool vec<N, T>::operator==(vec const &rhs) const {
    for (unsigned i = 0; i < N; i++) {
        if (!equals((*this)(i), rhs(i)))
            return false;
    }
    return true;
}

template <unsigned N, typename T> bool vec<N, T>::operator!=(vec const &rhs) const {
    return !(*this == rhs);
}

template <unsigned N, typename T> vec<N, T> vec<N, T>::operator-(vec const &rhs) const {
    vec<N, T> v;
    for (unsigned i = 0; i < N; i++)
        v(i) = (*this)(i)-rhs(i);
    return v;
}

template <unsigned N, typename T> vec<N, T> vec<N, T>::operator-() const {
    vec<N, T> v;
    for (unsigned i = 0; i < N; i++)
        v(i) = -(*this)(i);
    return v;
}

template <unsigned N, typename T> vec<N, T> vec<N, T>::operator*(T k) const {
    vec<N, T> v;
    for (unsigned i = 0; i < N; i++)
        v(i) = k * (*this)(i);
    return v;
}

template <unsigned N, typename T> vec<N, T> vec<N, T>::operator/(T k) const {
    vec<N, T> v;
    for (unsigned i = 0; i < N; i++)
        v(i) = (*this)(i) / k;
    return v;
}

template <unsigned N, typename T> inline vec<N, T> operator*(T k, vec<N, T> const &rhs) {
    return rhs * k;
}

template <unsigned N, typename T> T vec<N, T>::inner(vec const &rhs) const {
    T v = 0;
    for (unsigned i = 0; i < N; i++)
        v += (*this)(i)*rhs(i);
    return v;
}

template <unsigned N, typename T> vec<N, T> vec<N, T>::operator*(vec const &rhs) const {
    vec<N, T> v;
    for (unsigned i = 0; i < N; i++)
        v(i) = (*this)(i)*rhs(i);
    return v;
}

template <unsigned N, typename T> vec<N, T> vec<N, T>::operator/(vec const &rhs) const {
    vec<N, T> v;
    for (unsigned i = 0; i < N; i++)
        v(i) = (*this)(i) / rhs(i);
    return v;
}

template <unsigned N, typename T> vec<N, T> &vec<N, T>::operator+=(vec const &rhs) {
    for (unsigned i = 0; i < N; i++)
        (*this)(i) += rhs(i);
    return *this;
}

template <unsigned N, typename T> vec<N, T> &vec<N, T>::operator-=(vec const &rhs) {
    for (unsigned i = 0; i < N; i++)
        (*this)(i) -= rhs(i);
    return *this;
}

template <unsigned N, typename T> vec<N, T> &vec<N, T>::operator*=(vec const &rhs) {
    for (unsigned i = 0; i < N; i++)
        (*this)(i) *= rhs(i);
    return *this;
}

template <unsigned N, typename T> vec<N, T> vec<N, T>::outer(vec const &rhs) const {
    if (N != 3)
        throw std::string("outer product of 2 vectors only defines in 3 dimension.");
    else
        return vec<N, T>({(*this)(1) * rhs(2) - (*this)(2) * rhs(1),
                          (*this)(2) * rhs(0) - (*this)(0) * rhs(2),
                          (*this)(0) * rhs(1) - (*this)(1) * rhs(0)});
}

template <unsigned N, typename T> T vec<N, T>::norm2() const { return this->inner(*this); }

template <unsigned N, typename T> T vec<N, T>::norm() const { return std::sqrt(this->norm2()); }

template <unsigned N, typename T> vec<N, T> vec<N, T>::normalize() const {
    return 1.0f / norm() * (*this);
}

template <unsigned N, typename T> vec<N + 1, T> vec<N, T>::homo(T p) const {
    T v[N + 1];
    for (unsigned i = 0; i < N; i++)
        v[i] = (*this)(i);
    v[N] = p;
    return vec<N + 1, T>(v);
}

template <unsigned N, typename T> vec<N - 1, T> vec<N, T>::cart() const {
    T inv_w = 1.0f / (*this)(N - 1);

    T v[N - 1];
    for (unsigned i = 0; i < N - 1; i++)
        v[i] = inv_w * (*this)(i);
    return vec<N - 1, T>(v);
}

template <unsigned N, typename T> vec<N - 1, T> vec<N, T>::trunc() const {
    T v[N - 1];
    for (unsigned i = 0; i < N - 1; i++)
        v[i] = (*this)(i);
    return vec<N - 1, T>(v);
}

template <unsigned N, typename T> vec<N, T> vec<N, T>::at_least(T const &val) const {
    T v[N];
    for (unsigned i = 0; i < N; i++)
        v[i] = val > (*this)(i) ? val : (*this)(i);
    return vec<N, T>(v);
}

template <unsigned N, typename T> vec<N, T> vec<N, T>::at_most(T const &val) const {
    T v[N];
    for (unsigned i = 0; i < N; i++)
        v[i] = val < (*this)(i) ? val : (*this)(i);
    return vec<N, T>(v);
}

template <unsigned N, typename T>
inline std::ostream &operator<<(std::ostream &os, vec<N, T> const &v) {
    os << "(" << v(0);
    for (unsigned i = 1; i < N; i++) {
        os << "," << v(1);
    }
    os << ")";
    return os;
}

template <unsigned N, typename T> bool equals(vec<N, T> const &a, vec<N, T> const &b) {
    return a == b;
}

template <unsigned M, unsigned N, typename T = float> class mat {
  public:
    mat(std::initializer_list<T> const &v);
    mat(T v);
    mat();

    T operator()(unsigned i, unsigned j) const;
    T &operator()(unsigned i, unsigned j);
    mat operator+(mat const &rhs) const;
    mat operator-(mat const &rhs) const;
    mat<M, N, T> operator-() const;
    template <unsigned K> mat<M, K, T> operator*(mat<N, K, T> const &rhs) const;
    vec<N, T> operator*(vec<N, T> const &rhs) const;
    mat operator*(T k) const;
    mat operator^(int e) const;
    mat operator~() const;

    bool operator==(mat const &rhs) const;
    bool operator!=(mat const &rhs) const;

    void lu_decompose(mat &l, mat &u) const;
    vec<N, T> solve(vec<N, T> const &b) const;
    vec<N, T> ls_solve(vec<N, T> const &b) const;

    T det() const;
    T det(mat const &adj) const;
    mat adjugate() const;

  private:
    void lu_solve(mat const &l, mat const &u, vec<N, T> const &b, T *x) const;
    T det(unsigned *next_row, unsigned curr_col, unsigned removed_col) const;

    T e[N][M];
};

template <unsigned M, unsigned N, typename T> mat<M, N, T>::mat(std::initializer_list<T> const &v) {
    auto it = v.begin();
    for (unsigned j = 0; j < N; j++) {
        for (unsigned i = 0; i < M; i++) {
            (*this)(i, j) = *it;
            ++it;
        }
    }
}

template <unsigned M, unsigned N, typename T> mat<M, N, T>::mat(T v) {
    for (unsigned j = 0; j < N; j++) {
        for (unsigned i = 0; i < M; i++) {
            (*this)(i, j) = i == j ? v : 0;
        }
    }
}

template <unsigned M, unsigned N, typename T> mat<M, N, T>::mat() {
    for (unsigned j = 0; j < N; j++) {
        for (unsigned i = 0; i < M; i++) {
            (*this)(i, j) = 0;
        }
    }
}

template <unsigned M, unsigned N, typename T>
T mat<M, N, T>::operator()(unsigned i, unsigned j) const {
    return e[j][i];
}

template <unsigned M, unsigned N, typename T> T &mat<M, N, T>::operator()(unsigned i, unsigned j) {
    return e[j][i];
}

template <unsigned M, unsigned N, typename T>
mat<M, N, T> mat<M, N, T>::operator+(mat const &rhs) const {
    mat<M, N, T> m;
    for (unsigned j = 0; j < N; j++) {
        for (unsigned i = 0; i < M; i++) {
            m(i, j) = (*this)(i, j) + rhs(i, j);
        }
    }
    return m;
}

template <unsigned M, unsigned N, typename T>
mat<M, N, T> mat<M, N, T>::operator-(mat const &rhs) const {
    mat<M, N, T> m;
    for (unsigned j = 0; j < N; j++) {
        for (unsigned i = 0; i < M; i++) {
            m(i, j) = (*this)(i, j) - rhs(i, j);
        }
    }
    return m;
}

template <unsigned M, unsigned N, typename T> mat<M, N, T> mat<M, N, T>::operator-() const {
    mat<M, N, T> m;
    for (unsigned j = 0; j < N; j++) {
        for (unsigned i = 0; i < M; i++) {
            m(i, j) = -(*this)(i, j);
        }
    }
    return m;
}

template <unsigned M, unsigned N, typename T>
template <unsigned K>
mat<M, K, T> mat<M, N, T>::operator*(mat<N, K, T> const &rhs) const {
    mat<M, K, T> m;
    for (unsigned k = 0; k < K; k++) {
        for (unsigned j = 0; j < N; j++) {
            for (unsigned i = 0; i < M; i++) {
                m(i, k) = m(i, k) + (*this)(i, j) * rhs(j, k);
            }
        }
    }
    return m;
}

template <unsigned M, unsigned N, typename T>
vec<N, T> mat<M, N, T>::operator*(vec<N, T> const &rhs) const {
    vec<N, T> v;
    for (unsigned j = 0; j < N; j++) {
        for (unsigned i = 0; i < M; i++) {
            v(i) = v(i) + (*this)(i, j) * rhs(j);
        }
    }
    return v;
}

template <unsigned M, unsigned N, typename T> mat<M, N, T> mat<M, N, T>::operator*(T k) const {
    mat<M, N, T> m;
    for (unsigned j = 0; j < N; j++) {
        for (unsigned i = 0; i < M; i++) {
            m(i, j) = k * (*this)(i, j);
        }
    }
    return m;
}

template <unsigned M, unsigned N, typename T>
inline mat<M, N, T> operator*(T k, mat<M, N, T> const &rhs) {
    return rhs * k;
}

template <unsigned M, unsigned N, typename T>
T mat<M, N, T>::det(unsigned next_row[M + 1], unsigned curr_col, unsigned removed_col) const {
    unsigned curr_row = next_row[0];
    if (next_row[curr_row] == M + 1)
        return (*this)(curr_row - 1, curr_col - 1);

    T d = 0.0;
    for (unsigned i = 0, prev_row = 0; curr_row != M + 1;
         prev_row = curr_row, curr_row = next_row[curr_row], i++) {
        // 1. Remove row
        // 2. Compute minor
        // 3. Restore row
        next_row[prev_row] = next_row[curr_row];
        T m_ij = det(next_row, (curr_col + 1 == removed_col) ? (curr_col + 2) : (curr_col + 1),
                     removed_col);
        next_row[prev_row] = curr_row;

        // Compute cofactor.
        T a = (*this)(curr_row - 1, curr_col - 1);
        d += (i & 1 ? -a : a) * m_ij;
    }
    return d;
}

template <unsigned M, unsigned N, typename T> T mat<M, N, T>::det(mat const &adj) const {
    T d = 0;
    for (unsigned i = 0; i < M; i++) {
        d += (*this)(i, 0) * adj(0, i);
    }
    return d;
}

// Transposed comatrix.
template <unsigned M, unsigned N, typename T> mat<M, N, T> mat<M, N, T>::adjugate() const {
    unsigned next_row[M + 1];
    for (unsigned i = 0; i <= N; i++)
        next_row[i] = i + 1;

    mat adj;
    for (unsigned j = 1; j < N + 1; j++) {
        for (unsigned i = 1; i < M + 1; i++) {
            // 1. Remove row
            // 2. Compute minor
            // 3. Restore row
            next_row[i - 1] = i + 1;
            T m_ij = det(next_row, j == 1 ? 2 : 1, j);
            next_row[i - 1] = i;

            // Transpose and store the value.
            adj(j - 1, i - 1) = (i + j) & 1 ? -m_ij : m_ij;
        }
    }
    return adj;
}

template <unsigned M, unsigned N, typename T> T mat<M, N, T>::det() const {
    return det(adjugate());
}

template <unsigned M, unsigned N, typename T> mat<M, N, T> mat<M, N, T>::operator^(int e) const {
    if (M != N)
        throw std::string("needs to be a square matrix.");
    if (e == 0)
        return 1.0f;
    else if (e > 0) {
        mat m = (*this);
        for (int i = 1; i < e; i++)
            m = m * m;
        return m;
    } else {
        // Invert the matrix;
        mat m;

        if (N <= 4) {
            // Analytic solution.
            mat const &com = adjugate();
            T d = det(com);
            if (equals(d, 0))
                throw std::string("matrix not invertible.");
            m = T(1.0) / d * com;
        } else {
            // Baed on LU factorization.
            mat l, u;
            lu_decompose(l, u);
            vec<N, T> b;
            for (unsigned j = 0; j < N; j++) {
                b(j) = 1;
                lu_solve(l, u, b, &m(0, j));
                b(j) = 0;
            }
        }

        // Power.
        for (int i = -1; i > e; i--)
            m = m * m;
        return m;
    }
}

template <unsigned M, unsigned N, typename T> mat<M, N, T> mat<M, N, T>::operator~() const {
    mat m;
    for (unsigned j = 0; j < N; j++) {
        for (unsigned i = 0; i < M; i++) {
            m(j, i) = (*this)(i, j);
        }
    }
    return m;
}

template <unsigned M, unsigned N, typename T> bool mat<M, N, T>::operator==(mat const &rhs) const {
    for (unsigned j = 0; j < N; j++) {
        for (unsigned i = 0; i < M; i++) {
            if (!equals((*this)(i, j), rhs(i, j)))
                return false;
        }
    }
    return true;
}

template <unsigned M, unsigned N, typename T> bool mat<M, N, T>::operator!=(mat const &rhs) const {
    return !((*this) == rhs);
}

template <unsigned M, unsigned N, typename T>
void mat<M, N, T>::lu_decompose(mat &l, mat &u) const {
    if (M != N)
        throw std::string("Cannot perform LU decomposition on non-square matrix");
    for (unsigned i = 0; i < M; i++) {
        l(i, 0) = (*this)(i, 0);
    }
    for (unsigned j = 0; j < N; j++) {
        u(0, j) = (*this)(0, j) / l(0, 0);
    }
    for (unsigned j = 1; j < N - 1; j++) {
        for (unsigned i = j; i < M; i++) {
            T r = 0.0f;
            for (unsigned k = 0; k < j; k++) {
                r += l(i, k) * u(k, j);
            }
            l(i, j) = (*this)(i, j) - r;
        }
        for (unsigned i = j + 1; i < N; i++) {
            T r = 0.0f;
            for (unsigned k = 0; k < j; k++) {
                r += l(j, k) * u(k, i);
            }
            u(j, i) = ((*this)(j, i) - r) / l(j, j);
        }
        u(j, j) = 1;
    }
    T r = 0.0f;
    for (unsigned i = 0; i < N - 1; i++) {
        r += l(N - 1, i) * u(i, N - 1);
    }
    l(N - 1, N - 1) = (*this)(N - 1, N - 1) - r;
    u(N - 1, N - 1) = 1;
}

template <unsigned M, unsigned N, typename T>
vec<N, T> mat<M, N, T>::solve(vec<N, T> const &b) const {
    vec<N, T> x;
    mat l, u;
    lu_decompose(l, u);
    lu_solve(l, u, b, &x(0));
    return x;
}

template <unsigned M, unsigned N, typename T>
vec<N, T> mat<M, N, T>::ls_solve(vec<N, T> const &b) const {
    mat const &A = *this;
    mat const &A_T = ~A;
    vec<N, T> const &d = A_T * b;
    return (A_T * A).solve(d);
}

template <unsigned M, unsigned N, typename T>
void mat<M, N, T>::lu_solve(mat const &l, mat const &u, vec<N, T> const &b, T *x) const {
    vec<N, T> d;
    for (unsigned i = 0; i < M; i++) {
        T r = 0.0;
        for (unsigned k = 0; k < i; k++) {
            r += l(i, k) * d(k);
        }
        d(i) = (b(i) - r) / l(i, i);
    }

    for (int i = M - 1; i >= 0; i--) {
        T r = 0.0;
        for (unsigned k = i + 1; k < N; k++) {
            r += u(i, k) * x[k];
        }
        x[i] = d(i) - r;
    }
}

template <unsigned M, unsigned N, typename T>
inline std::ostream &operator<<(std::ostream &os, mat<M, N, T> const &m) {
    for (unsigned i = 0; i < M; i++) {
        os << m(i, 0);
        for (unsigned j = 1; j < N; j++) {
            os << "," << m(i, j);
        }
        os << std::endl;
    }
    return os;
}

typedef mat<2, 2> mat22;
typedef mat<3, 3> mat33;
typedef mat<4, 4> mat44;

typedef mat<2, 1> mat21;
typedef mat<3, 1> mat31;
typedef mat<4, 1> mat41;

class ray {
  public:
    ray();
    ray(vec3 const &m_o, vec3 const &m_v);
    bool intersect(vec3 const &v0, vec3 const &v1, vec3 const &v2, float t_min, float t_max,
                   vec3 &b, float &t0) const;

    vec3 o() const;
    vec3 v() const;
    vec3 v_inv() const;

  private:
    vec3 m_o;
    vec3 m_v;
    vec3 m_v_inv;
};

inline ray::ray() : m_o(0.0f), m_v(0.0f), m_v_inv(0.0f) {}

inline ray::ray(vec3 const &o, vec3 const &v) : m_o(o), m_v(v), m_v_inv(vec3(1.0f) / v) {}

inline bool ray::intersect(vec3 const &v0, vec3 const &v1, vec3 const &v2, float t_min, float t_max,
                           vec3 &b, float &t0) const {
    // b0 = 1 - b1 - b2
    // (1 - b1 - b2)*P0 + b1*P1 + b2*P2 = Pr0 + Vt
    // -Vx*t + b1*(P1x - P0x) + b2*(P2x - P0x) = Pr0x - P0x
    // substitution: va = P1 - P0, vb = P2 - P0, vc = Pr0 - P0
    vec3 const &va = v1 - v0;
    vec3 const &vb = v2 - v0;
    vec3 const &vc = m_o - v0;

    // solve for t, b1, b2, where
    // 0 <= b0 <= 1 and 0 <= b1 <= 1 and 0 <= b2 <= 1
    // or, b1 + b2 <= 1 and 0 <= b1 <= 1 and b2 => 0
    // which means that the intersect is within the triangle
    vec3 const &e2 = m_v.outer(vb);

    float det = va.inner(e2);
    if (det == 0.0f)
        return false;

    float inv = 1.0f / det;
    b(1) = vc.inner(e2) * inv;
    if (b(1) < 0.0f || b(1) > 1.0f)
        return false;

    vec3 const &e1 = vc.outer(va);
    b(2) = m_v.inner(e1) * inv;
    if (b(2) < 0.0f || b(1) + b(2) > 1.0f)
        return false;

    // within the ray ?
    t0 = vb.inner(e1) * inv;
    if (t0 < t_min || t0 > t_max)
        return false;

    b(0) = 1.0f - b(1) - b(2);
    return true;
}

inline vec3 ray::o() const { return m_o; }

inline vec3 ray::v() const { return m_v; }

inline vec3 ray::v_inv() const { return m_v_inv; }

class aabb {
  public:
    aabb();
    aabb(vec3 const &m_min, vec3 const &m_max);

    aabb operator+(aabb const &rhs) const;
    aabb operator+(vec3 const &rhs) const;
    aabb operator^(aabb const &rhs) const;

    bool is_empty() const;
    float surf_area() const;
    vec3 centroid() const;
    float enclosing_radius() const;
    bool intersect(ray const &r, float t_min, float t_max, float &t0, float &t1) const;

    vec3 min() const;
    vec3 max() const;

  private:
    vec3 m_min;
    vec3 m_max;
    bool m_is_empty;
};

inline aabb::aabb() : m_is_empty(true) {}

inline aabb::aabb(vec3 const &min, vec3 const &max) : m_min(min), m_max(max) {
    m_is_empty = min(0) > max(0) || min(1) > max(1) || min(2) > max(2);
}

inline aabb aabb::operator+(aabb const &rhs) const {
    if (!m_is_empty) {
        return aabb(vec3({std::min(m_min(0), rhs.m_min(0)), std::min(m_min(1), rhs.m_min(1)),
                          std::min(m_min(2), rhs.m_min(2))}),
                    vec3({std::max(m_max(0), rhs.m_max(0)), std::max(m_max(1), rhs.m_max(1)),
                          std::max(m_max(2), rhs.m_max(2))}));
    } else {
        return rhs;
    }
}

inline aabb aabb::operator+(vec3 const &rhs) const {
    if (!m_is_empty) {
        return aabb(vec3({std::min(m_min(0), rhs(0)), std::min(m_min(1), rhs(1)),
                          std::min(m_min(2), rhs(2))}),
                    vec3({std::max(m_max(0), rhs(0)), std::max(m_max(1), rhs(1)),
                          std::max(m_max(2), rhs(2))}));
    } else {
        return aabb(rhs, rhs);
    }
}

inline aabb aabb::operator^(aabb const &rhs) const {
    if (!m_is_empty) {
        return aabb(vec3({std::max(m_min(0), rhs.m_min(0)), std::max(m_min(1), rhs.m_min(1)),
                          std::max(m_min(2), rhs.m_min(2))}),
                    vec3({std::min(m_max(0), rhs.m_max(0)), std::min(m_max(1), rhs.m_max(1)),
                          std::min(m_max(2), rhs.m_max(2))}));
    } else {
        return aabb();
    }
}

inline vec3 aabb::min() const { return m_min; }

inline vec3 aabb::max() const { return m_max; }

inline bool aabb::is_empty() const { return m_is_empty; }

inline float aabb::surf_area() const {
    if (m_is_empty) {
        return 0.0f;
    } else {
        vec3 d = m_max - m_min;
        return 2.0f * (d(0) * d(1) + d(0) * d(2) + d(1) * d(2));
    }
}

inline float aabb::enclosing_radius() const { return (m_max - m_min).norm() / 2; }

inline vec3 aabb::centroid() const { return (m_max + m_min) / 2; }

inline bool aabb::intersect(ray const &r, float t_min, float t_max, float &t0, float &t1) const {
    vec3 const &o = r.o();
    vec3 const &v_inv = r.v_inv();

    // test against the plane parallels to x, y, z axis respectively
    for (unsigned i = 0; i < 3; i++) {
        float k0 = (m_min(i) - o(i)) * v_inv(i);
        float k1 = (m_max(i) - o(i)) * v_inv(i);

        // march toward the middle of the ray
        if (k0 > k1) {
            t_min = std::max(k1, t_min);
            t_max = std::min(k0, t_max);
        } else {
            t_min = std::max(k0, t_min);
            t_max = std::min(k1, t_max);
        }
        // out of the corner
        if (t_min > t_max) {
            return false;
        }
    }
    t0 = t_min;
    t1 = t_max;
    return true;
}

inline mat44 mat44_scale(vec3 const &s) {
    return mat44({s(0), 0, 0, 0, 0, s(1), 0, 0, 0, 0, s(2), 0, 0, 0, 0, 1});
}

inline mat44 mat44_translate(vec3 const &v) {
    return mat44({1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, v(0), v(1), v(2), 1});
}

inline mat44 mat44_basis(vec3 const &u, vec3 const &v, vec3 const &w) {
    return mat44({u(0), u(1), u(2), 0, v(0), v(1), v(2), 0, w(0), w(1), w(2), 0, 0, 0, 0, 1});
}

inline mat44 mat44_rotate(float a, vec3 const &axis) {
    vec3 const &w = axis.normalize();

    vec3 const &ref = std::abs(w(1) - 1) < 1e-5 ? vec3({1, 0, 0}) : vec3({0, 1, 0});

    vec3 const &u = w.outer(ref).normalize();
    vec3 const &v = w.outer(u).normalize();

    mat44 const &inv_r = mat44_basis(u, v, w);
    mat44 const &r = ~inv_r;

    float cos = std::cos(a);
    float sin = std::sin(a);

    mat44 const &rotw = mat44({cos, sin, 0, 0, -sin, cos, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1});

    return inv_r * rotw * r;
}

inline mat44 mat44_lookat(vec3 const &eye, vec3 const &center, vec3 const &up) {
    vec3 const &w = (center - eye).normalize();
    vec3 const &u_up = up.normalize();
    vec3 const &v = w.outer(u_up).normalize();
    vec3 const &u = v.outer(w);

    mat44 const &l =
        mat44({v(0), u(0), -w(0), 0, v(1), u(1), -w(1), 0, v(2), u(2), -w(2), 0, 0, 0, 0, 1});

    mat44 const &t = mat44_translate(-eye);
    return t * l;
}

class frustum {
  public:
    frustum(float left, float right, float bottom, float top, float z_near, float z_far);

    mat44 projective_transform();

  private:
    float left;
    float right;
    float top;
    float bottom;
    float z_near;
    float z_far;
};

inline frustum::frustum(float left, float right, float top, float bottom, float z_near, float z_far)
    : left(left), right(right), top(top), bottom(bottom), z_near(z_near), z_far(z_far) {}

inline mat44 frustum::projective_transform() {
    float a = 2 * z_near;
    float width = right - left;
    float height = top - bottom;
    float d = z_far - z_near;

    float delta_y = top + bottom;
    float delta_x = left + right;

    return mat44({a / width, 0, 0, 0, 0, a / height, 0, 0, delta_x / width, delta_y / height,
                  (-z_far - z_near) / d, -1, 0, 0, -a * z_far / d, 0});
}

inline frustum frustum_perspective2(float tan_fovy, float aspect, float z_near, float z_far) {
    float tan = tan_fovy;
    float top = z_near * tan;
    float right = top * aspect;
    return frustum(-right, right, top, -top, z_near, z_far);
}

inline frustum frustum_perspective(float fovy, float aspect, float z_near, float z_far) {
    return frustum_perspective2(std::tan(fovy), aspect, z_near, z_far);
}

inline mat44 mat44_viewport(float x, float y, float height, float width) {
    return mat44(
        {width / 2, 0, 0, 0, 0, height / 2, 0, 0, 0, 0, 1, 0, width / 2 + x, height / 2 + y, 0, 1});
}

inline mat44 mat44_normal(mat44 const &affine) { return ~(affine ^ (-1)); }

inline float rad2deg(float rad) { return rad / M_PI * 180; }

inline float deg2rad(float deg) { return deg / 180 * M_PI; }

inline void vec3_basis(vec3 const &n, vec3 &u, vec3 &v) {
    u = ((std::abs(n(0)) > .1 ? vec3({0.0f, 1.0f, 0.0f}) : vec3({1.0f, 1.0f, 1.0f})).outer(n))
            .normalize();
    v = n.outer(u);
}

inline vec3 vec3_cos_hemisphere_sample(vec3 const &n, float e0, float e1) {
    e8util::vec3 u, v;
    e8util::vec3_basis(n, u, v);

    float phi = e0 * 2 * M_PI;

    float z = std::sqrt(e1);
    float r = std::sqrt(1.0f - z * z);
    float x = r * std::cos(phi);
    float y = r * std::sin(phi);

    return x * u + y * v + z * n;
}

class rng {
  public:
    rng(unsigned int seed) : engine(seed), ur(0, 1) {}

    rng() : ur(0, 1) {
        std::random_device rd;
        engine.seed(rd());
    }

    float draw() { return ur(engine); }

  private:
    std::default_random_engine engine;
    std::uniform_real_distribution<float> ur;
};

#define CLAMP(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))

} // namespace e8util

#endif // TENSOR_H
