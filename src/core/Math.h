#pragma once

namespace math
{
    static constexpr float PI = 3.14159265358979323846f;
    static constexpr float TWO_PI = 6.28318530717958647692f;

    // Returns a number in the range [0, 1)
    inline static float Randf() { return rand() / (RAND_MAX + 1.0f); }
    inline static float Rad2Deg(float radians) { return radians * 180.0f / PI; }
    inline static float Deg2Rad(float degrees) { return degrees * PI / 180.0f; }
    inline static float Fast_Min(float a, float b) { return a < b ? a : b; }
    inline static float Fast_Max(float a, float b) { return a > b ? a : b; }
    inline static int Fast_Min(int a, int b) { return a < b ? a : b; }
    inline static int Fast_Max(int a, int b) { return a > b ? a : b; }
    inline static bool FloatEq(float a, float b) { return fabsf(a - b) <= FLT_EPSILON; }
    inline static float Clampf(float value, float min_value, float max_value) { return Fast_Max(min_value, Fast_Min(value, max_value)); }
    inline static int Clamp(int value, int min_value, int max_value) { return Fast_Max(min_value, Fast_Min(value, max_value)); }
    inline static float Map(float value, float in_min, float in_max, float out_min, float out_max) {
        return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }
}

class Vector2
{
public:
    Vector2() : x(0), y(0) {};
    Vector2(float val) : x(val), y(val) {};
    Vector2(float _x, float _y) : x(_x), y(_y) {};
    ~Vector2() = default;

    Vector2& operator=(float value);
    Vector2& operator=(Vector2 const& other);
    bool operator<(float value) const;
    bool operator>(float value) const;
    bool operator==(float value) const;
    bool operator==(Vector2 const& other) const;
    bool operator!=(Vector2 const& other) const;
    bool operator!=(float value) const;

    Vector2 operator+(Vector2 const& other) const;
    Vector2 operator-(Vector2 const& other) const;
    friend Vector2 operator-(Vector2 const& other);
    Vector2 operator*(float value) const;
    friend Vector2 operator*(float value, Vector2 const& other);
    Vector2 operator/(float value) const;
    Vector2& operator-=(const Vector2& other);
    Vector2& operator+=(const Vector2& other);
    Vector2& operator*=(float value);
    Vector2& operator/=(float value);

    float operator*(Vector2 const& other) const; // Dot product

    float const& operator[](unsigned int i) const { return e[i]; }
    float& operator[](unsigned int i) { return e[i]; }

    float length();
    float lengthSquared();
    Vector2& normalize();
    float dot(Vector2 const& other) const;

    void set(float _x, float _y) { x = _x; y = _y; }

    union
    {
        struct
        {
            float x, y;
        };
        struct
        {
            float e[2];
        };
    };
};

class Vector3
{
public:
    Vector3() : x(0), y(0), z(0) {};
    Vector3(float val) : x(val), y(val), z(val){};
    Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {};
    Vector3(Vector2 const& v) : x(v.x), y(v.y), z(0) {};
    ~Vector3() = default;

    operator Vector2() const { return Vector2(x, y); }

    Vector3& operator=(float value);
    Vector3& operator=(Vector3 const& other);
    bool operator<(float value) const;
    bool operator>(float value) const;
    bool operator==(float value) const;
    bool operator==(Vector3 const& other) const;
    bool operator!=(Vector3 const& other) const;
    bool operator!=(float value) const;

    Vector3 operator+(Vector3 const& other) const;
    Vector3 operator-(Vector3 const& other) const;
    friend Vector3 operator-(Vector3 const& other);
    Vector3 operator*(float value) const;
    friend Vector3 operator*(float value, Vector3 const& other);
    Vector3 operator/(float value) const;
    Vector3& operator-=(const Vector3& other);
    Vector3& operator+=(const Vector3& other);
    Vector3& operator*=(float value);
    Vector3& operator/=(float value);

    float operator*(Vector3 const& other) const; // Dot product
    Vector3 operator^(Vector3 const& other) const; // Cross product

    float const& operator[](unsigned int i) const { return e[i]; }
    float& operator[](unsigned int i) { return e[i]; }

    float length() const;
    float lengthSquared() const;
    Vector3& normalize();
    Vector3 getNormalized() const;
    float dot(Vector3 const& other) const;
    Vector3 cross(Vector3 const& other) const;
    
    union
    {
        struct
        {
            float x, y, z;
        };
        struct
        {
            float e[3];
        };
    };
};

class Vector4
{
public:
    Vector4() : x(0), y(0), z(0), w(0) {};
    Vector4(float val) : x(val), y(val), z(val), w(val) {};
    Vector4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {};
    Vector4(Vector2 const& v) : x(v.x), y(v.y), z(0), w(0) {};
    Vector4(Vector3 const& v) : x(v.x), y(v.y), z(v.z), w(0) {};
    Vector4(Vector3 const& v, float _w) : x(v.x), y(v.y), z(v.z), w(_w) {};
    ~Vector4() = default;

    operator Vector2() const { return Vector2(x, y); }
    operator Vector3() const { return Vector3(x, y, z); }

    Vector4& operator=(float value);
    Vector4& operator=(Vector4 const& other);
    bool operator<(float value) const;
    bool operator>(float value) const;
    bool operator==(float value) const;
    bool operator==(Vector4 const& other) const;
    bool operator!=(Vector4 const& other) const;
    bool operator!=(float value) const;

    Vector4 operator+(Vector4 const& other) const;
    Vector4 operator-(Vector4 const& other) const;
    friend Vector4 operator-(Vector4 const& other);
    Vector4 operator*(float value) const;
    friend Vector4 operator*(float value, Vector4 const& other);
    Vector4 operator/(float value) const;
    Vector4& operator-=(const Vector4& other);
    Vector4& operator+=(const Vector4& other);
    Vector4& operator*=(float value);
    Vector4& operator/=(float value);

    float operator*(Vector4 const& other) const; // Dot product

    float const& operator[](unsigned int i) const { return e[i]; }
    float& operator[](unsigned int i) { return e[i]; }

    float length() const;
    float lengthSquared() const;
    Vector4& normalize();
    Vector4 getNormalized() const;
    float dot(Vector4 const& other) const;

    union
    {
        struct
        {
            float x, y, z, w;
        };
        struct
        {
            float e[4];
        };
    };
};

class Matrix3
{
public:
    Vector3 col0, col1, col2;
};

class Matrix4
{
public:
    Matrix4() : col0(0), col1(0), col2(0), col3(0) {};
    Matrix4(float val) : col0(val), col1(val), col2(val), col3(val) {};
    Matrix4(Vector4 _col0, Vector4 _col1, Vector4 _col2, Vector4 _col3) : col0(_col0), col1(_col1), col2(_col2), col3(_col3) {};
    ~Matrix4() = default;

    Matrix4& operator=(float value);
    Matrix4& operator=(Matrix4 const& other);
    bool operator==(Matrix4 const& other) const;
    bool operator!=(Matrix4 const& other) const;

    Matrix4 operator+(Matrix4 const& other) const;
    Matrix4 operator-(Matrix4 const& other) const;
    Matrix4 operator*(float value) const;
    friend Matrix4 operator*(float value, Matrix4 const& other);
    Vector4 operator*(Vector4 const& other) const;
    Matrix4 operator*(Matrix4 const& other) const;
    Matrix4& operator-=(Matrix4 const& other);
    Matrix4& operator+=(Matrix4 const& other);
    Matrix4& operator*=(float value);
    Matrix4& operator*=(Matrix4 const& other);

    Vector4 const& operator[](unsigned int i) const { return e[i]; }
    Vector4& operator[](unsigned int i) { return e[i]; }

    Matrix4& invert();
    Matrix4 getInverse() const;
    Matrix4& transpose();
    Matrix4 getTransposed() const;
    void setUpper3x3(Matrix3 const& upperMatrix);
    void setTranslation(Vector3 const& translation);

    static Matrix4 identity();
    static Matrix4 lookAt(Vector3 const& eyePosition, Vector3 const& direction, Vector3 const& up);
    static Matrix4 scale(Vector3 const& scaleVec);
    static Matrix4 translation(Vector3 const& translateVec);
    static Matrix4 perspective(float fieldOfViewVertical, float aspectRatio, float zNear, float zFar);
    static Matrix4 orthographic(float left, float right, float bottom, float top, float zNear, float zFar);
    static Matrix4 orthographic(float width, float height, float zNear, float zFar);

    union
    {
        struct
        {
            Vector4 col0, col1, col2, col3;
        };
        struct
        {
            Vector4 e[4];
        };
    };
};

class Quaternion
{
public:
    Quaternion() : elem() {};
    Quaternion(float val) : elem(val) {};
    Quaternion(Vector3 imaginary, float real) : elem(imaginary, real) {};
    Quaternion(Vector4 vec) : elem(vec) {};
    Quaternion(float x, float y, float z, float w) : elem(x,y,z,w) {};
    ~Quaternion() = default;

    operator Matrix4() const;

    Quaternion& operator=(float value);
    Quaternion& operator=(Quaternion const& other);
    bool operator==(Quaternion const& other) const;
    bool operator!=(Quaternion const& other) const;

    Quaternion operator+(Quaternion const& other) const;
    Quaternion operator-(Quaternion const& other) const;
    friend Quaternion operator-(Quaternion const& other);
    Quaternion operator*(float value) const;
    friend Quaternion operator*(float value, Quaternion const& other);
    Quaternion operator*(Quaternion const& other) const;
    Quaternion operator/(Quaternion const& other) const;
    Quaternion operator/(float value) const;
    Quaternion& operator-=(Quaternion const& other);
    Quaternion& operator+=(Quaternion const& other);
    Quaternion& operator*=(float value);
    Quaternion& operator*=(Quaternion const& other);
    Quaternion& operator/=(float value);

    float getReal() const;
    Vector3 getImaginary() const;
    float getAngle() const;
    Vector3 getAxis() const;
    Vector3 getEulerAngles() const;
    float getMagnitude() const;
    float getMagnitudeSquared() const;

    Quaternion& normalize();
    Quaternion getNormalized() const;
    Quaternion& conjugate();
    Quaternion getConjugate() const;
    Quaternion& invert();
    Quaternion getInverse() const;

    Vector3 apply(Vector3 const& vec) const;
    Quaternion slerp(Quaternion const& other, float t) const;
    Quaternion& rotate180AroundWorldX();
    Quaternion& rotate180AroundWorldY();
    Quaternion& rotate180AroundWorldZ();
    Quaternion& rotate180AroundLocalX();
    Quaternion& rotate180AroundLocalY();
    Quaternion& rotate180AroundLocalZ();
    
    static Quaternion slerp(Quaternion const& q1, Quaternion const& q2, float t);
    static Quaternion rotation(Vector3 axis, float radians);
    static Quaternion rotationX(float radians);
    static Quaternion rotationY(float radians);
    static Quaternion rotationZ(float radians);

private:
    union
    {
        struct
        {
            float x, y, z, w;
        };
        struct
        {
            Vector4 elem;
        };
    };
};

typedef Vector2 float2;
typedef Vector3 float3;
typedef Vector4 float4;
typedef Matrix3 float3x3;
typedef Matrix4 float4x4;