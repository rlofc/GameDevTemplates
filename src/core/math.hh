#ifndef GDT_MATH_HEADER_INCLUDED
#define GDT_MATH_HEADER_INCLUDED
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ostream>
#include "easing.hh"
namespace gdt {
namespace math {
static const float PI = 3.14159265358979323846f;

#ifndef max
float max(float x, float y);
#endif
#ifndef min
float min(float x, float y);
#endif
namespace templates {

static int rawcast(float x)
{
    union {
        float f;
        int i;
    } u;
    u.f = x;
    return u.i;
}

template <typename T>
struct vec2 {
    T x;
    T y;
    vec2() : vec2(T(0), T(0))
    {
    }
    vec2(const T* arr) : vec2(T(arr[0]), T(arr[1]))
    {
    }
    vec2(T x, T y) : x(x), y(y)
    {
    }
    T length_sqrd() const
    {
        T length = T(0);
        length += x * x;
        length += y * y;
        return length;
    }
    vec2& operator=(const vec2& v)
    {
        this->x = v.x;
        this->y = v.y;
        return *this;
    }
    T length() const
    {
        return sqrt(length_sqrd());
    }

    float dot(vec2 v2)
    {
        return (x * v2.x) + (y * v2.y);
    }
    vec2<T> normalize() const
    {
        T len = length();
        if (len == T(0)) {
            return vec2<T>();
        }
        else {
            return *this / len;
        }
    }
    vec2<T>& operator/=(T fac)
    {
        x = x / fac;
        y = y / fac;
        return *this;
    }
    friend vec2<T> operator/(vec2<T> lhs, T rhs)
    {
        return lhs /= rhs;
    }

    vec2<T>& operator*=(T fac)
    {
        x = x * fac;
        y = y * fac;
        return *this;
    }
    friend vec2<T> operator*(vec2<T> lhs, T rhs)
    {
        return lhs *= rhs;
    }
    vec2<T>& operator+=(const vec2<T>& rhs)
    {
        x = x + rhs.x;
        y = y + rhs.y;
        return *this;
    }
    friend vec2<T> operator+(vec2<T> lhs, const vec2<T>& rhs)
    {
        return lhs += rhs;
    }
    vec2<T>& operator-=(const vec2<T>& rhs)
    {
        x = x - rhs.x;
        y = y - rhs.y;
        return *this;
    }
    friend vec2<T> operator-(vec2<T> lhs, const vec2<T>& rhs)
    {
        return lhs -= rhs;
    }
    int hash() const
    {
        return abs(rawcast(x) ^ rawcast(y));
    }
};

template <typename T>
T randomize_in_range(T range)
{
    return static_cast<T>(rand()) / (static_cast<T>(RAND_MAX / range)) - range / 2;
}

template <typename T>
struct vec3 {
    T x;
    T y;
    T z;
    vec3() : vec3(T(0), T(0), T(0))
    {
    }
    vec3(const vec3 & v) : vec3(v.x, v.y, v.z) {}
    vec3& operator=(const vec3& v)
    {
        this->x = v.x;
        this->y = v.y;
        this->z = v.z;
        return *this;
    }
    vec3(const T* arr) : vec3(T(arr[0]), T(arr[1]), T(arr[2]))
    {
    }
    vec3(T x, T y, T z) : x(x), y(y), z(z)
    {
    }
    T length_sqrd() const
    {
        T length = T(0);
        length += x * x;
        length += y * y;
        length += z * z;
        return length;
    }
    T length() const
    {
        return sqrt(length_sqrd());
    }

    float dot(vec3 v2)
    {
        return (x * v2.x) + (y * v2.y) + (z * v2.z);
    }

    vec3<T> normalize() const
    {
        T len = length();
        if (len == T(0)) {
            return vec3<T>();
        }
        else {
            return *this / len;
        }
    }
    vec3<T> cross(vec3<T> v2) const
    {
        vec3<T> v;
        v.x = (y * v2.z) - (z * v2.y);
        v.y = (z * v2.x) - (x * v2.z);
        v.z = (x * v2.y) - (y * v2.x);
        return v;
    }
    vec3<T>& operator/=(T fac)
    {
        x = x / fac;
        y = y / fac;
        z = z / fac;
        return *this;
    }
    friend vec3<T> operator/(vec3<T> lhs, T rhs)
    {
        return lhs /= rhs;
    }

    vec3<T>& operator+=(T fac)
    {
        x = x + fac;
        y = y + fac;
        z = z + fac;
        return *this;
    }
    friend vec3<T> operator+(vec3<T> lhs, T rhs)
    {
        return lhs += rhs;
    }

    vec3<T>& operator*=(T fac)
    {
        x = x * fac;
        y = y * fac;
        z = z * fac;
        return *this;
    }

    vec3<T>& operator*=(vec3<T> rhs)
    {
        x = x * rhs.x;
        y = y * rhs.y;
        z = z * rhs.z;
        return *this;
    }
    friend vec3<T> operator*(vec3<T> lhs, T rhs)
    {
        return lhs *= rhs;
    }
    friend vec3<T> operator*(vec3<T> lhs, vec3<T> rhs)
    {
        return lhs *= rhs;
    }
    vec3<T>& operator+=(const vec3<T>& rhs)
    {
        x = x + rhs.x;
        y = y + rhs.y;
        z = z + rhs.z;
        return *this;
    }
    friend vec3<T> operator+(vec3<T> lhs, const vec3<T>& rhs)
    {
        return lhs += rhs;
    }
    vec3<T>& operator-=(const vec3<T>& rhs)
    {
        x = x - rhs.x;
        y = y - rhs.y;
        z = z - rhs.z;
        return *this;
    }
    friend vec3<T> operator-(vec3<T> lhs, const vec3<T>& rhs)
    {
        return lhs -= rhs;
    }
    static vec3<T> random(T rx, T ry, T rz);
    int hash() const
    {
        return abs(rawcast(x) ^ rawcast(y) ^ rawcast(z));
    }

    static vec3<T> slerp(vec3<T> v1, vec3<T> v2, float t)
    {
        float omega = acos((v1.cross(v1.normalize())).dot(v2.cross(v2.normalize())));
        float so = sinf(omega);
        return (v1 * (sinf((1.0 - t) * omega) / so)) + (v2 * (sinf(t * omega) / so));
    }

    static vec3<T> lerp(vec3<T> v1, vec3<T> v2, float t)
    {
        vec3<T> res;
        res.x = interpolate(v1.x, v2.x, t, linear_interpolation);
        res.y = interpolate(v1.y, v2.y, t, linear_interpolation);
        res.z = interpolate(v1.z, v2.z, t, linear_interpolation);
        return res;
    }
};

template <typename T>
vec3<T> vec3<T>::random(T rx, T ry, T rz)
{
    vec3<T> r;
    r.x = randomize_in_range<T>(rx);
    r.y = randomize_in_range<T>(ry);
    r.z = randomize_in_range<T>(rz);
    return r;
}

template <typename T>
struct vec4 {
    T x;
    T y;
    T z;
    T w;
    vec4() : vec4(T(0), T(0), T(0), T(0))
    {
    }
    vec4(const T* arr) : vec4(T(arr[0]), T(arr[1]), T(arr[2]), T(arr[3]))
    {
    }
    vec4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w)
    {
    }

    T length_sqrd() const
    {
        T length = T(0);
        length += x * x;
        length += y * y;
        length += z * z;
        length += w * w;
        return length;
    }
    T length() const
    {
        return sqrt(length_sqrd());
    }
    vec4<T> normalize() const
    {
        T len = length();
        if (len == T(0)) {
            return vec4<T>();
        }
        else {
            return *this / len;
        }
    }
    vec4<T>& operator/=(T fac)
    {
        x = x / fac;
        y = y / fac;
        z = z / fac;
        w = w / fac;
        return *this;
    }
    friend vec4<T> operator/(vec4<T> lhs, T rhs)
    {
        return lhs /= rhs;
    }

    vec4<T>& operator*=(T fac)
    {
        x = x * fac;
        y = y * fac;
        z = z * fac;
        w = w * fac;
        return *this;
    }
    friend vec4<T> operator*(vec4<T> lhs, T rhs)
    {
        return lhs *= rhs;
    }
    vec4<T>& operator+=(const vec4<T>& rhs)
    {
        x = x + rhs.x;
        y = y + rhs.y;
        z = z + rhs.z;
        w = w + rhs.z;
        return *this;
    }
    friend vec4<T> operator+(vec4<T> lhs, const vec4<T>& rhs)
    {
        return lhs += rhs;
    }
    vec4<T>& operator-=(const vec4<T>& rhs)
    {
        x = x - rhs.x;
        y = y - rhs.y;
        z = z - rhs.z;
        w = w - rhs.z;
        return *this;
    }
    friend vec4<T> operator-(vec4<T> lhs, const vec4<T>& rhs)
    {
        return lhs -= rhs;
    }
    int hash() const
    {
        return abs(rawcast(x) ^ rawcast(y) ^ rawcast(z) ^ rawcast(w));
    }
};

template <typename T>
struct quat : vec4<T> {
    quat() : quat(T(0), T(0), T(0), T(0))
    {
    }
    quat(T x, T y, T z, T w) : vec4<T>(x, y, z, w)
    {
    }
    quat(vec4<T> v) : vec4<T>(v)
    {
    }
    quat(vec3<T> eular)
    {
        float fc1 = cosf(eular.z / 2.0f);
        float fc2 = cosf(eular.x / 2.0f);
        float fc3 = cosf(eular.y / 2.0f);

        float fs1 = sinf(eular.z / 2.0f);
        float fs2 = sinf(eular.x / 2.0f);
        float fs3 = sinf(eular.y / 2.0f);

        this->x = fc1 * fc2 * fs3 - fs1 * fs2 * fc3;
        this->y = fc1 * fs2 * fc3 + fs1 * fc2 * fs3;
        this->z = fs1 * fc2 * fc3 - fc1 * fs2 * fs3;
        this->w = fc1 * fc2 * fc3 + fs1 * fs2 * fs3;
    };

    quat(float angle, vec3<T> axis)
    {
        float sine = sinf(angle / 2.0f);
        float cosine = cosf(angle / 2.0f);
        *this = vec4<T>(axis.x * sine, axis.y * sine, axis.z * sine, cosine).normalize();
    }

    T real() const
    {
        return this->w;
    }
    vec3<T> imaginaries() const
    {
        return vec3<T>(this->x, this->y, this->z);
    }
    vec3<T> eular() const
    {
        float sqrx = this->x * this->x;
        float sqry = this->y * this->y;
        float sqrz = this->z * this->z;
        float sqrw = this->w * this->w;

        return vec3<T>(asinf(-2.0f * (this->x * this->z - this->y * this->w)),
                       atan2f(2.0f * (this->y * this->z + this->x * this->w),
                              (-sqrx - sqry + sqrz + sqrw)),
                       atan2f(2.0f * (this->x * this->y + this->z * this->w),
                              (sqrx - sqry - sqrz + sqrw)));
    }

    static quat<T> identity()
    {
        return quat<T>(0, 0, 0, 1);
    }

    vec3<T> rotate(vec3<T> v) const
    {
        // nVidia SDK implementation
        vec3<T> uv, uuv;
        vec3<T> qvec(this->x, this->y, this->z);
        uv = qvec.cross(v);
        uuv = qvec.cross(uv);
        uv *= (2.0f * this->w);
        uuv *= 2.0f;
        return v + uv + uuv;
    }

    static quat<T> slerp(quat<T> from, quat<T> to, float amount)
    {
        float scale0, scale1;
        float afto1[4];
        float cosom = from.x * to.x + from.y * to.y + from.z * to.z + from.w * to.w;

        if (cosom < 0.0f) {
            cosom = -cosom;
            afto1[0] = -to.x;
            afto1[1] = -to.y;
            afto1[2] = -to.z;
            afto1[3] = -to.w;
        }
        else {
            afto1[0] = to.x;
            afto1[1] = to.y;
            afto1[2] = to.z;
            afto1[3] = to.w;
        }

        const float QUATERNION_DELTA_COS_MIN = 0.01f;

        if ((1.0f - cosom) > QUATERNION_DELTA_COS_MIN) {
            /* This is a standard case (slerp). */
            float omega = acosf(cosom);
            float sinom = sinf(omega);
            scale0 = sinf((1.0f - amount) * omega) / sinom;
            scale1 = sinf(amount * omega) / sinom;
        }
        else {
            /* "from" and "to" quaternions are very close */
            /*  so we can do a linear interpolation.      */
            scale0 = 1.0f - amount;
            scale1 = amount;
        }
        return quat<T>((scale0 * from.x) + (scale1 * afto1[0]),
                       (scale0 * from.y) + (scale1 * afto1[1]),
                       (scale0 * from.z) + (scale1 * afto1[2]),
                       (scale0 * from.w) + (scale1 * afto1[3]));
    }
};

template <typename T>
struct quat_dual {
    quat<T> real;
    quat<T> dual;
};

template <typename T>
bool operator==(const vec2<T>& v1, const vec2<T>& v2)
{
    if (!(v1.x == v2.x)) {
        return false;
    }
    if (!(v1.y == v2.y)) {
        return false;
    }
    return true;
}

template <typename T>
bool operator==(const vec3<T>& v1, const vec3<T>& v2)
{
    if (!(v1.x == v2.x)) {
        return false;
    }
    if (!(v1.y == v2.y)) {
        return false;
    }
    if (!(v1.z == v2.z)) {
        return false;
    }
    return true;
}
template <typename T>
bool operator==(const vec4<T>& v1, const vec4<T>& v2)
{
    if (!(v1.x == v2.x)) {
        return false;
    }
    if (!(v1.y == v2.y)) {
        return false;
    }
    if (!(v1.z == v2.z)) {
        return false;
    }
    if (!(v1.w == v2.w)) {
        return false;
    }
    return true;
}

template <typename T>
struct mat3 {
    T xx;
    T xy;
    T xz;
    T yx;
    T yy;
    T yz;
    T zx;
    T zy;
    T zz;
    // clang-format off
    mat3() : mat3(T(0), T(0), T(0), 
                  T(0), T(0), T(0),
                  T(0), T(0), T(0)) {}

    mat3(const mat3<T> & m) : mat3(
        m.xx, m.xy, m.xz, 
        m.yx, m.yy, m.yz, 
        m.zx, m.zy, m.zz ) {}

    mat3(T xx, T xy, T xz, 
         T yx, T yy, T yz, 
         T zx, T zy, T zz  ) : xx(xx), xy(xy), xz(xz),
                               yx(yx), yy(yy), yz(yz),
                               zx(zx), zy(zy), zz(zz) {}

    float det() const {
  return (this->xx * this->yy * this->zz) + (this->xy * this->yz * this->zx) + (this->xz * this->yx * this->zy) -
         (this->xz * this->yy * this->zx) - (this->xy * this->yx * this->zz) - (this->xx * this->yz * this->zy);

    }
};

template <typename T>
struct mat4 {
    T xx;
    T xy;
    T xz;
    T xw;
    T yx;
    T yy;
    T yz;
    T yw;
    T zx;
    T zy;
    T zz;
    T zw;
    T wx;
    T wy;
    T wz;
    T ww;
    // clang-format off
    mat4() : mat4(T(0), T(0), T(0), T(0), 
                  T(0), T(0), T(0), T(0),
                  T(0), T(0), T(0), T(0),
                  T(0), T(0), T(0), T(0)) {}

    mat4(const mat4<T> & m) : mat4(
        m.xx, m.xy, m.xz, m.xw,
        m.yx, m.yy, m.yz, m.yw,
        m.zx, m.zy, m.zz, m.zw,
        m.wx, m.wy, m.wz, m.ww) {}

    mat4(T xx, T xy, T xz, T xw,
         T yx, T yy, T yz, T yw,
         T zx, T zy, T zz, T zw,
         T wx, T wy, T wz, T ww) : xx(xx), xy(xy), xz(xz), xw(xw),
                                   yx(yx), yy(yy), yz(yz), yw(yw),
                                   zx(zx), zy(zy), zz(zz), zw(zw),
                                   wx(wx), wy(wy), wz(wz), ww(ww) {}

    T at(int x, int y) const {
      T* arr = (T*)(this);
      return arr[x + (y*4)];  
    }


    quat<T> as_quat() const {
          float tr = this->xx + this->yy + this->zz;

          if (tr > 0.0f) {
            
            float s = sqrtf( tr + 1.0f );
            
            float w = s / 2.0f;
            float x = ( this->at(1, 2) - this->at(2, 1) ) * (0.5f / s);
            float y = ( this->at(2, 0) - this->at(0, 2) ) * (0.5f / s);
            float z = ( this->at(0, 1) - this->at(1, 0) ) * (0.5f / s);
            return quat<T>(x, y, z, w);
            
          } else {
            
            int nxt[3] = {1, 2, 0};
            float q[4];
            int  i, j, k;
            
            i = 0;
            if ( this->at(1, 1) > this->at(0, 0) ) {	i = 1;	}
            if ( this->at(2, 2) > this->at(i, i) ) {	i = 2;	}
            j = nxt[i];
            k = nxt[j];

            float s = sqrtf( (this->at(i, i) - (this->at(j, j) + this->at(k, k))) + 1.0f );

            q[i] = s * 0.5f;

            if ( s != 0.0f )	{	s = 0.5f / s;	}

            q[3] = ( this->at(j, k) - this->at(k, j) ) * s;
            q[j] = ( this->at(i, j) + this->at(j, i) ) * s;
            q[k] = ( this->at(i, k) + this->at(k, i) ) * s;

            return quat<T>(q[0], q[1], q[2], q[3]);
          }
    }
    quat_dual<T> as_quat_dual() const {
        quat<T> q= this->as_quat();
        vec3<T> t = *this * vec3<T>();
        quat_dual<T> ret;
        ret.real = q;
        ret.dual = quat<T>(
             0.5 * ( t.x * q.w + t.y * q.z - t.z * q.y),
             0.5 * (-t.x * q.z + t.y * q.w + t.z * q.x),
             0.5 * ( t.x * q.y - t.y * q.x + t.z * q.w),
            -0.5 * ( t.x * q.x + t.y * q.y + t.z * q.z)
        );
        return ret;
    }
    // clang-format on
    mat4<T> transpose() const
    {
        mat4<T> mat;

        mat.xx = xx;
        mat.xy = yx;
        mat.xz = zx;
        mat.xw = wx;

        mat.yx = xy;
        mat.yy = yy;
        mat.yz = zy;
        mat.yw = wy;

        mat.zx = xz;
        mat.zy = yz;
        mat.zz = zz;
        mat.zw = wz;

        mat.wx = xw;
        mat.wy = yw;
        mat.wz = zw;
        mat.ww = ww;

        return mat;
    }
    // clang-format on

    float det() const
    {
        float cofact_xx = mat3<T>(this->yy, this->yz, this->yw, this->zy, this->zz, this->zw,
                                  this->wy, this->wz, this->ww)
                              .det();
        float cofact_xy = -mat3<T>(this->yx, this->yz, this->yw, this->zx, this->zz, this->zw,
                                   this->wx, this->wz, this->ww)
                               .det();
        float cofact_xz = mat3<T>(this->yx, this->yy, this->yw, this->zx, this->zy, this->zw,
                                  this->wx, this->wy, this->ww)
                              .det();
        float cofact_xw = -mat3<T>(this->yx, this->yy, this->yz, this->zx, this->zy, this->zz,
                                   this->wx, this->wy, this->wz)
                               .det();

        return (cofact_xx * this->xx) + (cofact_xy * this->xy) + (cofact_xz * this->xz) +
               (cofact_xw * this->xw);
    }

    mat4<T> inverse() const
    {
        float det = this->det();
        float fac = 1.0 / det;

        mat4 ret;
        ret.xx = fac * mat3<T>(yy, yz, yw, zy, zz, zw, wy, wz, ww).det();
        ret.xy = fac * -mat3<T>(yx, yz, yw, zx, zz, zw, wx, wz, ww).det();
        ret.xz = fac * mat3<T>(yx, yy, yw, zx, zy, zw, wx, wy, ww).det();
        ret.xw = fac * -mat3<T>(yx, yy, yz, zx, zy, zz, wx, wy, wz).det();

        ret.yx = fac * -mat3<T>(xy, xz, xw, zy, zz, zw, wy, wz, ww).det();
        ret.yy = fac * mat3<T>(xx, xz, xw, zx, zz, zw, wx, wz, ww).det();
        ret.yz = fac * -mat3<T>(xx, xy, xw, zx, zy, zw, wx, wy, ww).det();
        ret.yw = fac * mat3<T>(xx, xy, xz, zx, zy, zz, wx, wy, wz).det();

        ret.zx = fac * mat3<T>(xy, xz, xw, yy, yz, yw, wy, wz, ww).det();
        ret.zy = fac * -mat3<T>(xx, xz, xw, yx, yz, yw, wx, wz, ww).det();
        ret.zz = fac * mat3<T>(xx, xy, xw, yx, yy, yw, wx, wy, ww).det();
        ret.zw = fac * -mat3<T>(xx, xy, xz, yx, yy, yz, wx, wy, wz).det();

        ret.wx = fac * -mat3<T>(xy, xz, xw, yy, yz, yw, zy, zz, zw).det();
        ret.wy = fac * mat3<T>(xx, xz, xw, yx, yz, yw, zx, zz, zw).det();
        ret.wz = fac * -mat3<T>(xx, xy, xw, yx, yy, yw, zx, zy, zw).det();
        ret.ww = fac * mat3<T>(xx, xy, xz, yx, yy, yz, zx, zy, zz).det();

        ret = ret.transpose();

        return ret;
    }

    mat4<T>& operator*=(const mat4<T>& m2)
    {
        T _xx = (xx * m2.xx) + (xy * m2.yx) + (xz * m2.zx) + (xw * m2.wx);
        T _xy = (xx * m2.xy) + (xy * m2.yy) + (xz * m2.zy) + (xw * m2.wy);
        T _xz = (xx * m2.xz) + (xy * m2.yz) + (xz * m2.zz) + (xw * m2.wz);
        T _xw = (xx * m2.xw) + (xy * m2.yw) + (xz * m2.zw) + (xw * m2.ww);
        xx = _xx;
        xy = _xy;
        xz = _xz;
        xw = _xw;

        T _yx = (yx * m2.xx) + (yy * m2.yx) + (yz * m2.zx) + (yw * m2.wx);
        T _yy = (yx * m2.xy) + (yy * m2.yy) + (yz * m2.zy) + (yw * m2.wy);
        T _yz = (yx * m2.xz) + (yy * m2.yz) + (yz * m2.zz) + (yw * m2.wz);
        T _yw = (yx * m2.xw) + (yy * m2.yw) + (yz * m2.zw) + (yw * m2.ww);
        yx = _yx;
        yy = _yy;
        yz = _yz;
        yw = _yw;

        T _zx = (zx * m2.xx) + (zy * m2.yx) + (zz * m2.zx) + (zw * m2.wx);
        T _zy = (zx * m2.xy) + (zy * m2.yy) + (zz * m2.zy) + (zw * m2.wy);
        T _zz = (zx * m2.xz) + (zy * m2.yz) + (zz * m2.zz) + (zw * m2.wz);
        T _zw = (zx * m2.xw) + (zy * m2.yw) + (zz * m2.zw) + (zw * m2.ww);
        zx = _zx;
        zy = _zy;
        zz = _zz;
        zw = _zw;

        T _wx = (wx * m2.xx) + (wy * m2.yx) + (wz * m2.zx) + (ww * m2.wx);
        T _wy = (wx * m2.xy) + (wy * m2.yy) + (wz * m2.zy) + (ww * m2.wy);
        T _wz = (wx * m2.xz) + (wy * m2.yz) + (wz * m2.zz) + (ww * m2.wz);
        T _ww = (wx * m2.xw) + (wy * m2.yw) + (wz * m2.zw) + (ww * m2.ww);
        wx = _wx;
        wy = _wy;
        wz = _wz;
        ww = _ww;

        return *this;
    }


    friend mat4<T> operator*(mat4<T> lhs, const mat4<T>& rhs)
    {
        return lhs *= rhs;
    }

    friend vec4<T> operator*(mat4<T> lhs, const vec4<T>& v)
    {
      vec4<T> vec;
      
      vec.x = (lhs.xx * v.x) + (lhs.xy * v.y) + (lhs.xz * v.z) + (lhs.xw * v.w);
      vec.y = (lhs.yx * v.x) + (lhs.yy * v.y) + (lhs.yz * v.z) + (lhs.yw * v.w);
      vec.z = (lhs.zx * v.x) + (lhs.zy * v.y) + (lhs.zz * v.z) + (lhs.zw * v.w);
      vec.w = (lhs.wx * v.x) + (lhs.wy * v.y) + (lhs.wz * v.z) + (lhs.ww * v.w);
      
      return vec;
    }
    friend vec3<T> operator*(mat4<T> lhs, const vec3<T>& v)
    {
        vec4<T> vh = vec4<T>(v.x, v.y, v.z, 1);
        vh = lhs * vh;
        vh = vh / vh.w;
        return vec3<T>(vh.x, vh.y, vh.z);
    }

    static mat4<T> id();
    static mat4<T> perspective(T fov, T near_clip, T far_clip, T ratio);
    static mat4<T> ortho(T left, T right, T bottom, T top, T clip_near, T clip_far);
    static mat4<T> translation(vec3<T> v);
    static mat4<T> scale(vec3<T> v);
    static mat4<T> rotation_quat(vec4<T> q);
    static mat4<T> world(vec3<T> pos, vec3<T> scale, vec4<T> q);
    static mat4<T> view_look_at(vec3<T> position, vec3<T> target, vec3<T> up);
    static mat4<T> rotation_eular(vec3<T> v);
};

template <typename T>
mat4<T> mat4<T>::id()
{
    return mat4<T>(T(1), T(0), T(0), T(0), T(0), T(1), T(0), T(0), T(0), T(0), T(1), T(0), T(0),
                   T(0), T(0), T(1));
}

template <typename T>
mat4<T> mat4<T>::perspective(T fov, T near_clip, T far_clip, T ratio)
{
    mat4<T> m;
    float right, left, bottom, top;

    right = -(near_clip * tanf(fov));
    left = -right;

    top = ratio * near_clip * tanf(fov);
    bottom = -top;

    m.xx = (2.0 * near_clip) / (right - left);
    m.yy = (2.0 * near_clip) / (top - bottom);
    m.xz = (right + left) / (right - left);
    m.yz = (top + bottom) / (top - bottom);
    m.zz = (-far_clip - near_clip) / (far_clip - near_clip);
    m.wz = -1.0;
    m.zw = (-(2.0 * near_clip) * far_clip) / (far_clip - near_clip);
    return m;
}

template <typename T>
mat4<T> mat4<T>::ortho(T left, T right, T bottom, T top, T clip_near, T clip_far)
{
    mat4 m = mat4<T>::id();

    m.xx = 2 / (right - left);
    m.yy = 2 / (top - bottom);
    m.zz = 1 / (clip_near - clip_far);

    m.xw = -1 - 2 * left / (right - left);
    m.yw = 1 + 2 * top / (bottom - top);
    m.zw = clip_near / (clip_near - clip_far);

    return m;
}

template <typename T>
mat4<T> mat4<T>::translation(vec3<T> v)
{
    auto m = mat4<T>::id();
    m.xw = v.x;
    m.yw = v.y;
    m.zw = v.z;
    return m;
}

template <typename T>
mat4<T> mat4<T>::scale(vec3<T> v)
{
    auto m = mat4<T>::id();
    m.xx = v.x;
    m.yy = v.y;
    m.zz = v.z;
    return m;
}

template <typename T>
mat4<T> mat4<T>::rotation_quat(vec4<T> q)
{
    float x2 = q.x + q.x;
    float y2 = q.y + q.y;
    float z2 = q.z + q.z;
    float xx = q.x * x2;
    float yy = q.y * y2;
    float wx = q.w * x2;
    float xy = q.x * y2;
    float yz = q.y * z2;
    float wy = q.w * y2;
    float xz = q.x * z2;
    float zz = q.z * z2;
    float wz = q.w * z2;

    return mat4<T>(1.0f - (yy + zz), xy - wz, xz + wy, 0.0f, xy + wz, 1.0f - (xx + zz), yz - wx,
                   0.0f, xz - wy, yz + wx, 1.0f - (xx + yy), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
}

template <typename T>
mat4<T> mat4<T>::world(vec3<T> pos, vec3<T> scale, vec4<T> q)
{
    mat4<T> pos_m, sca_m, rot_m, result;

    pos_m = mat4<T>::translation(pos);
    rot_m = mat4<T>::rotation_quat(q);
    sca_m = mat4<T>::scale(scale);

    result = mat4<T>::id();
    result = result * pos_m;
    result = result * rot_m;
    result = result * sca_m;

    return result;
}

template <typename T>
mat4<T> mat4<T>::view_look_at(vec3<T> position, vec3<T> target, vec3<T> up)
{
    auto m = mat4<T>::id();
    vec3<T> zaxis = (target - position).normalize();
    vec3<T> xaxis = up.cross(zaxis).normalize();
    vec3<T> yaxis = zaxis.cross(xaxis);

    m.xx = xaxis.x;
    m.xy = xaxis.y;
    m.xz = xaxis.z;

    m.yx = yaxis.x;
    m.yy = yaxis.y;
    m.yz = yaxis.z;

    m.zx = -zaxis.x;
    m.zy = -zaxis.y;
    m.zz = -zaxis.z;

    return m *= mat4<T>::translation(position * -1);
}

template <typename T>
mat4<T> mat4<T>::rotation_eular(vec3<T> v)
{
    mat4<T> m = mat4<T>();

    float cosx = cos(v.x);
    float cosy = cos(v.y);
    float cosz = cos(v.z);
    float sinx = sin(v.x);
    float siny = sin(v.y);
    float sinz = sin(v.z);

    m.xx = cosy * cosz;
    m.yx = -cosx * sinz + sinx * siny * cosz;
    m.zx = sinx * sinz + cosx * siny * cosz;

    m.xy = cosy * sinz;
    m.yy = cosx * cosz + sinx * siny * sinz;
    m.zy = -sinx * cosz + cosx * siny * sinz;

    m.xz = -siny;
    m.yz = sinx * cosy;
    m.zz = cosx * cosy;

    m.ww = 1;

    return m;
}

template <typename T>
static quat<T> rotation_to(vec3<T> src, vec3<T> dest, vec3<T> fallbackAxis = vec3<T>())
{
    // Based on Stan Melax's article in Game Programming Gems
    quat<T> q;
    // Copy, since cannot modify local
    vec3<T> v0 = src;
    vec3<T> v1 = dest;
    v0.normalize();
    v1.normalize();
    T d = v0.dot(v1);
    // If dot == 1, vectors are the same
    if (d >= 1.0f) {
        return quat<T>::identity();
    }
    if (d < (1e-6f - 1.0f)) {
        if ((fallbackAxis == vec3<T>()) == false) {
            // rotate 180 degrees about the fallback axis
            q = quat<T>(3.14159265358797f, fallbackAxis);
        }
        else {
            // Generate an axis
            vec3<T> axis = vec3<T>(1, 0, 0).cross(src);
            if (axis.length() == 0.0)  // pick another if colinear
                axis = vec3<T>(0, 1, 0).cross(src);
            axis.normalize();
            q = quat<T>(3.14159265358797f, axis);
        }
    }
    else {
        T s = sqrtf((1 + d) * 2);
        T invs = 1 / s;
        vec3<T> c = v0.cross(v1);
        q.x = c.x * invs;
        q.y = c.y * invs;
        q.z = c.z * invs;
        q.w = s * 0.5f;
        q.normalize();
    }
    return q;
}
};
using vec2i = templates::vec2<uint32_t>;
using vec2 = templates::vec2<float>;
using vec3 = templates::vec3<float>;
using vec4 = templates::vec4<float>;
using mat4 = templates::mat4<float>;
using quat = templates::quat<float>;
void vec2i_to_array(vec2i v, uint32_t* out);
void vec2_to_array(vec2 v, float* out);
void vec3_to_array(vec3 v, float* out);
void vec4_to_array(vec4 v, float* out);
void mat4_to_array(mat4 m, float* out);
void quat_to_array(quat v, float* out);

};
};

std::ostream& operator<<(std::ostream& os, const gdt::math::vec2& v);

std::ostream& operator<<(std::ostream& os, const gdt::math::vec2i& v);

std::ostream& operator<<(std::ostream& os, const gdt::math::vec3& v);

std::ostream& operator<<(std::ostream& os, const gdt::math::vec4& v);
#endif  // GDT_MATH_HEADER_INCLUDED
