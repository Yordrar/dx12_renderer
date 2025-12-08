#include "Math.h"

#include <DirectXMath.h>

inline Vector2& Vector2::operator=(float value)
{
    x = value;
    y = value;

    return *this;
}

inline Vector2& Vector2::operator=(const Vector2& other)
{
    x = other.x;
    y = other.y;

    return *this;
}

bool Vector2::operator<(float value) const
{
    return x < value && y < value;
}

bool Vector2::operator>(float value) const
{
    return x > value && y > value;
}

bool Vector2::operator==(float value) const
{
    return x == value && y == value;
}

bool Vector2::operator==(const Vector2& other) const
{
    return x == other.x && y == other.y;
}

bool Vector2::operator!=(const Vector2& other) const
{
    return !(*this == other);
}

bool Vector2::operator!=(float value) const
{
    return !(*this == value);
}

Vector2 Vector2::operator+(Vector2 const& other) const
{
    return Vector2(x + other.x, y + other.y);
}

Vector2 Vector2::operator-(Vector2 const& other) const
{
    return Vector2(x - other.x, y - other.y);
}

Vector2 operator-(Vector2 const& other)
{
    return Vector2(-other.x, -other.y);
}

Vector2 Vector2::operator*(float value) const
{
    return Vector2(x * value, y * value);
}

Vector2 operator*(float value, const Vector2& other)
{
    return Vector2(other.x * value, other.y * value);
}

Vector2 Vector2::operator/(float value) const
{
    return Vector2(x / value, y / value);
}

Vector2& Vector2::operator-=(const Vector2& other)
{
    x -= other.x;
    y -= other.y;
    return *this;
}

Vector2& Vector2::operator+=(const Vector2& other)
{
    x += other.x;
    y += other.y;
    return *this;
}

Vector2& Vector2::operator*=(float value)
{
    x *= value;
    x *= value;
    return *this;
}

Vector2& Vector2::operator/=(float value)
{
    x /= value;
    x /= value;
    return *this;
}

float Vector2::operator*(Vector2 const& other) const
{
    return x * other.x + y * other.y;
}

float Vector2::length()
{
    return sqrtf(x * x + y * y);
}

float Vector2::lengthSquared()
{
    return x * x + y * y;
}

Vector2& Vector2::normalize()
{
    *this /= length();
    return *this;
}

float Vector2::dot(Vector2 const& other) const
{
    return *this * other;
}




Vector3& Vector3::operator=(float value)
{
    x = value;
    y = value;
    z = value;
    return *this;
}

Vector3& Vector3::operator=(Vector3 const& other)
{
    x = other.x;
    y = other.y;
    z = other.z;
    return *this;
}

bool Vector3::operator<(float value) const
{
    return x < value && y < value && z < value;
}

bool Vector3::operator>(float value) const
{
    return x > value && y > value && z > value;
}

bool Vector3::operator==(float value) const
{
    return x == value && y == value && z == value;
}

bool Vector3::operator==(Vector3 const& other) const
{
    return x == other.x && y == other.y && z == other.z;
}

bool Vector3::operator!=(Vector3 const& other) const
{
    return !(*this == other);
}

bool Vector3::operator!=(float value) const
{
    return !(*this == value);
}

Vector3 Vector3::operator+(Vector3 const& other) const
{
    return Vector3(x + other.x, y + other.y, z + other.z);
}

Vector3 Vector3::operator-(Vector3 const& other) const
{
    return Vector3(x - other.x, y - other.y, z - other.z);
}

Vector3 operator-(Vector3 const& other)
{
    return Vector3(-other.x, -other.y, -other.z);
}

Vector3 Vector3::operator*(float value) const
{
    return Vector3(x * value, y * value, z * value);
}

Vector3 operator*(float value, Vector3 const& other)
{
    return other * value;
}

Vector3 Vector3::operator/(float value) const
{
    return Vector3(x / value, y / value, z / value);
}

Vector3& Vector3::operator-=(const Vector3& other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

Vector3& Vector3::operator+=(const Vector3& other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

Vector3& Vector3::operator*=(float value)
{
    x *= value;
    y *= value;
    z *= value;
    return *this;
}

Vector3& Vector3::operator/=(float value)
{
    x /= value;
    y /= value;
    z /= value;
    return *this;
}

// Dot product
float Vector3::operator*(Vector3 const& other) const
{
    return this->dot(other);
}
// Cross product
Vector3 Vector3::operator^(Vector3 const& other) const
{
    return this->cross(other);
}

float Vector3::length() const
{
    return sqrtf(x * x + y * y + z * z);
}

float Vector3::lengthSquared() const
{
    return x * x + y * y + z * z;
}

Vector3& Vector3::normalize()
{
    *this /= length();
    return *this;
}

Vector3 Vector3::getNormalized() const
{
    Vector3 vec(*this);
    return vec / vec.length();
}

float Vector3::dot(Vector3 const& other) const
{
    return x * other.x + y * other.y + z * other.z;
}

Vector3 Vector3::cross(Vector3 const& other) const
{
    return Vector3(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    );
}





Vector4& Vector4::operator=(float value)
{
    x = value;
    y = value;
    z = value;
    w = value;
    return *this;
}

Vector4& Vector4::operator=(Vector4 const& other)
{
    x = other.x;
    y = other.y;
    z = other.z;
    w = other.w;
    return *this;
}

bool Vector4::operator<(float value) const
{
    return x < value && y < value && z < value && w < value;
}

bool Vector4::operator>(float value) const
{
    return x > value && y > value && z > value && w > value;
}

bool Vector4::operator==(float value) const
{
    return x == value && y == value && z == value && w == value;
}

bool Vector4::operator==(Vector4 const& other) const
{
    return x == other.x && y == other.y && z == other.z && w == other.w;
}

bool Vector4::operator!=(Vector4 const& other) const
{
    return !(*this == other);
}

bool Vector4::operator!=(float value) const
{
    return !(*this == value);
}

Vector4 Vector4::operator+(Vector4 const& other) const
{
    return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
}

Vector4 Vector4::operator-(Vector4 const& other) const
{
    return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
}

Vector4 operator-(Vector4 const& other)
{
    return Vector4(-other.x, -other.y, -other.z, -other.w);
}

Vector4 Vector4::operator*(float value) const
{
    return Vector4(x * value, y * value, z * value, w * value);
}

Vector4 operator*(float value, Vector4 const& other)
{
    return other * value;
}

Vector4 Vector4::operator/(float value) const
{
    return Vector4(x / value, y / value, z / value, w / value);
}

Vector4& Vector4::operator-=(const Vector4& other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;
    return *this;
}

Vector4& Vector4::operator+=(const Vector4& other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;
    return *this;
}

Vector4& Vector4::operator*=(float value)
{
    x *= value;
    y *= value;
    z *= value;
    w *= value;
    return *this;
}

Vector4& Vector4::operator/=(float value)
{
    x /= value;
    y /= value;
    z /= value;
    w /= value;
    return *this;
}

// Dot product
float Vector4::operator*(Vector4 const& other) const
{
    return this->dot(other);
}

float Vector4::length() const
{
    return sqrtf(x * x + y * y + z * z + w * w);
}

float Vector4::lengthSquared() const
{
    return x * x + y * y + z * z + w * w;
}

Vector4& Vector4::normalize()
{
    *this /= length();
    return *this;
}

Vector4 Vector4::getNormalized() const
{
    Vector4 vec(*this);
    return vec / vec.length();
}

float Vector4::dot(Vector4 const& other) const
{
    return x * other.x + y * other.y + z * other.z + w * other.w;
}









Matrix4& Matrix4::operator=(float value)
{
    col0 = value;
    col1 = value;
    col2 = value;
    col3 = value;
    return *this;
}

Matrix4& Matrix4::operator=(Matrix4 const& other)
{
    col0 = other.col0;
    col1 = other.col1;
    col2 = other.col2;
    col3 = other.col3;
    return *this;
}

bool Matrix4::operator==(Matrix4 const& other) const
{
    return col0 == other.col0 && col1 == other.col1 && col2 == other.col2 && col3 == other.col3;
}

bool Matrix4::operator!=(Matrix4 const& other) const
{
    return !(*this == other);
}

Matrix4 Matrix4::operator+(Matrix4 const& other) const
{
    return Matrix4(
        col0 + other.col0,
        col1 + other.col1, 
        col2 + other.col2, 
        col3 + other.col3
        );
}

Matrix4 Matrix4::operator-(Matrix4 const& other) const
{
    return Matrix4(
        col0 - other.col0,
        col1 - other.col1,
        col2 - other.col2,
        col3 - other.col3
    );
}

Matrix4 Matrix4::operator*(float value) const
{
    return Matrix4(
        col0 * value,
        col1 * value,
        col2 * value,
        col3 * value
    );
}

Matrix4 operator*(float value, Matrix4 const& other)
{
    return Matrix4(
        other.col0 * value,
        other.col1 * value,
        other.col2 * value,
        other.col3 * value
    );
}

Vector4 Matrix4::operator*(Vector4 const& other) const
{
    Vector4 result(col0 * other.x + col1 * other.y + col2 * other.z + col3 * other.w );
    return result;
}

Matrix4 Matrix4::operator*(Matrix4 const& other) const
{
    Matrix4 result(
        *this * other.col0,
        *this * other.col1,
        *this * other.col2,
        *this * other.col3
    );
    return result;
}

Matrix4& Matrix4::operator-=(Matrix4 const& other)
{
    col0 -= other.col0;
    col1 -= other.col1;
    col2 -= other.col2;
    col3 -= other.col3;
    return *this;
}

Matrix4& Matrix4::operator+=(Matrix4 const& other)
{
    col0 += other.col0;
    col1 += other.col1;
    col2 += other.col2;
    col3 += other.col3;
    return *this;
}

Matrix4& Matrix4::operator*=(float value)
{
    col0 *= value;
    col1 *= value;
    col2 *= value;
    col3 *= value;
    return *this;
}

Matrix4& Matrix4::operator*=(Matrix4 const& other)
{
    *this = *this * other;
    return *this;
}

Matrix4& Matrix4::invert()
{
    auto matrix = DirectX::XMMatrixSet(
        e[0][0], e[1][0], e[2][0], e[3][0],
        e[0][1], e[1][1], e[2][1], e[3][1],
        e[0][2], e[1][2], e[2][2], e[3][2],
        e[0][3], e[1][3], e[2][3], e[3][3]
    );

    if (DirectX::XMMatrixDeterminant(matrix).m128_f32[0] == 0.0f)
    {
        return *this; // Return a ref to the original untouched matrix if no inverse
    }

    matrix = DirectX::XMMatrixInverse(nullptr, matrix);

    memcpy(this, &matrix, sizeof(*this));
    return this->transpose();
}

Matrix4 Matrix4::getInverse() const
{
    Matrix4 mat(*this);
    mat.invert();
    return mat;
}

Matrix4& Matrix4::transpose()
{
    Matrix4 transposed = this->getTransposed();
    *this = transposed;
    return *this;
}

Matrix4 Matrix4::getTransposed() const
{
    return Matrix4(
        Vector4(col0.x, col1.x, col2.x, col3.x),
        Vector4(col0.y, col1.y, col2.y, col3.y),
        Vector4(col0.z, col1.z, col2.z, col3.z),
        Vector4(col0.w, col1.w, col2.w, col3.w)
    );
}

void Matrix4::setUpper3x3(Matrix3 const& upperMatrix)
{
    col0 = Vector4(upperMatrix.col0, 0);
    col1 = Vector4(upperMatrix.col1, 0);
    col2 = Vector4(upperMatrix.col2, 0);
}

void Matrix4::setTranslation(Vector3 const& translation)
{
    col3.x = translation.x;
    col3.y = translation.y;
    col3.z = translation.z;
}

Matrix4 Matrix4::identity()
{
    return Matrix4(
        Vector4(1,0,0,0),
        Vector4(0,1,0,0),
        Vector4(0,0,1,0),
        Vector4(0,0,0,1)
    );
}

Matrix4 Matrix4::lookAt(Vector3 const& position, Vector3 const& direction, Vector3 const& up)
{
    Vector3 normalizedUp = up.getNormalized();
    Vector3 normalizedDirection = direction.getNormalized();
    Vector3 right = normalizedDirection.cross(normalizedUp);
    Vector3 localUp = right.cross(normalizedDirection);

    Matrix4 lookAtMatrix(
        right.normalize(),
        localUp.normalize(),
        -normalizedDirection,
        Vector4(position, 1)
    );

    return lookAtMatrix.invert();
}

Matrix4 Matrix4::scale(Vector3 const& scaleVec)
{
    return Matrix4(
        Vector4(scaleVec.x, 0, 0, 0),
        Vector4(0, scaleVec.y, 0, 0),
        Vector4(0, 0, scaleVec.z, 0),
        Vector4(0, 0, 0, 1)
    );
}

Matrix4 Matrix4::translation(Vector3 const& translateVec)
{
    return Matrix4(
        Vector4(1, 0, 0, 0),
        Vector4(0, 1, 0, 0),
        Vector4(0, 0, 1, 0),
        Vector4(translateVec, 1)
    );
}

Matrix4 Matrix4::perspective(float fieldOfViewVertical, float aspectRatio, float zNear, float zFar)
{
    float tanHalfFov = tanf(math::Deg2Rad(fieldOfViewVertical / 2.0f));
    return Matrix4(
        Vector4(1.0f / (tanHalfFov*aspectRatio),0,0,0),
        Vector4(0, 1.0f / tanHalfFov,0,0),
        Vector4(0.0f, 0.0f, zFar/(zNear-zFar), -1.0f),
        Vector4(0.0f, 0.0f, (zNear*zFar)/(zNear-zFar), 0.0f)
    );
}

Matrix4 Matrix4::orthographic(float left, float right, float bottom, float top, float zNear, float zFar)
{
    return Matrix4(
        Vector4(2.0f/(right-left), 0, 0, 0),
        Vector4(0, 2.0f/(top-bottom), 0, 0),
        Vector4(0, 0, 1.0f/(zNear-zFar), 0),
        Vector4(-(right+left)/(right-left), -(top+bottom)/(top-bottom), zNear/(zFar-zNear), 1)
    );
}

Matrix4 Matrix4::orthographic(float width, float height, float zNear, float zFar)
{
    float halfWidth = width * 0.5f;
    float halfHeight = height * 0.5f;
    return Matrix4::orthographic(-halfWidth, halfWidth, -halfHeight, halfHeight, zNear, zFar);
}









Quaternion::operator Matrix4() const
{
    Matrix4 matrix = Matrix4::identity();
    matrix[0][0] = 2 * (x * x + y * y) - 1;
    matrix[0][1] = 2 * (y * z + x * w);
    matrix[0][2] = 2 * (y * w - x * z);
    matrix[1][0] = 2 * (y * z - x * w);
    matrix[1][1] = 2 * (x * x + z * z) - 1;
    matrix[1][2] = 2 * (z * w + x * y);
    matrix[2][0] = 2 * (y * w + x * z);
    matrix[2][1] = 2 * (z * w - x * y);
    matrix[2][2] = 2 * (x * x + w * w) - 1;
    return matrix;
}

Quaternion& Quaternion::operator=(float value)
{
    elem = value;
    return *this;
}

Quaternion& Quaternion::operator=(Quaternion const& other)
{
    elem = other.elem;
    return *this;
}

bool Quaternion::operator==(Quaternion const& q) const
{
    return x == q.x && y == q.y && z == q.z && w == q.w;
}

bool Quaternion::operator!=(Quaternion const& q) const
{
    return !(*this == q);
}

Quaternion Quaternion::operator+(Quaternion const& other) const
{
    return Quaternion(x + other.x, y + other.y, z + other.z, w + other.w);
}
Quaternion Quaternion::operator-(Quaternion const& other) const
{
    return Quaternion(x - other.x, y - other.y, z - other.z, w - other.w);
}

Quaternion operator-(Quaternion const& other)
{
    return Quaternion(-other.x, -other.y, -other.z, -other.w);
}

Quaternion Quaternion::operator*(Quaternion const& other) const
{
    Vector3 thisImaginary(x, y, z);
    Vector3 otherImaginary = other.getImaginary();
    return Quaternion(
        w * otherImaginary + other.w * thisImaginary + thisImaginary.cross(otherImaginary),
        w * other.w - thisImaginary.dot(otherImaginary));
}

Quaternion Quaternion::operator*(float val) const
{
    return Quaternion(x * val, y * val, z * val, w * val);
}

Quaternion operator*(float value, Quaternion const& other)
{
    return other * value;
}

Quaternion Quaternion::operator/(Quaternion const& other) const
{
    return *this * other.getInverse();
}

Quaternion Quaternion::operator/(float val) const
{
    float invVal = 1.0f / val;
    return Quaternion(x * invVal, y * invVal, z * invVal, w * invVal);
}

Quaternion& Quaternion::operator-=(Quaternion const& other)
{
    elem -= other.elem;
    return *this;
}

Quaternion& Quaternion::operator+=(Quaternion const& other)
{
    elem += other.elem;
    return *this;
}

Quaternion& Quaternion::operator*=(float value)
{
    elem *= value;
    return *this;
}

Quaternion& Quaternion::operator*=(Quaternion const& other)
{
    *this = *this * other;
    return *this;
}

Quaternion& Quaternion::operator/=(float value)
{
    elem /= value;
    return *this;
}

float Quaternion::getReal() const
{
    return w;
}

Vector3 Quaternion::getImaginary() const
{
    return Vector3(elem);
}

float Quaternion::getAngle() const
{
    return 2.0f * acosf(fminf(w, 1.0f));
}

Vector3 Quaternion::getAxis() const
{
    Vector3 axis(elem);
    axis.normalize();

    if (w < 0.0f)
    {
        axis = -axis;
    }

    return axis;
}

Vector3 Quaternion::getEulerAngles() const
{
    Vector3 eulerAngles;

    // pitch (x-axis rotation)
    float sinr_cosp = 2 * (w * x + y * z);
    float cosr_cosp = 1 - 2 * (x * x + y * y);
    eulerAngles.x = std::atan2(sinr_cosp, cosr_cosp);

    // yaw (y-axis rotation)
    float siny = 2 * (w * y - z * x);
    if (std::abs(siny) >= 1)
        eulerAngles.y = static_cast<float>(std::copysign(math::PI / 2, siny)); // use 90 degrees if out of range
    else
        eulerAngles.y = std::asin(siny);

    // roll (z-axis rotation)
    float siny_cosp = 2 * (w * z + x * y);
    float cosy_cosp = 1 - 2 * (y * y + z * z);
    eulerAngles.z = std::atan2(siny_cosp, cosy_cosp);

    return eulerAngles;
}

float Quaternion::getMagnitude() const
{
    return elem.length();
}

float Quaternion::getMagnitudeSquared() const
{
    return elem.lengthSquared();
}

Quaternion& Quaternion::normalize()
{
    elem /= elem.length();
    return *this;
}

Quaternion Quaternion::getNormalized() const
{
    Quaternion q(*this);
    return q / q.getMagnitude();
}

Quaternion& Quaternion::conjugate()
{
    elem = -elem;
    w = -w;
    return *this;
}

Quaternion Quaternion::getConjugate() const
{
    return Quaternion(-x, -y, -z, w);
}

Quaternion& Quaternion::invert()
{
    *this /= getMagnitudeSquared();
    return this->conjugate();
}

Quaternion Quaternion::getInverse() const
{
    Quaternion q(*this);
    q /= q.getMagnitudeSquared();
    return q.conjugate();
}

Vector3 Quaternion::apply(Vector3 const& vec) const
{
    Quaternion p(vec, 0.0f);
    return (*this * p * getConjugate()).getImaginary();
}

Quaternion Quaternion::slerp(Quaternion const& other, float t) const
{
    return Quaternion::slerp(*this, other, t);
}

Quaternion& Quaternion::rotate180AroundWorldX()
{
    elem = Vector4(w, -z, y, -x);
    return *this;
}

Quaternion& Quaternion::rotate180AroundWorldY()
{
    elem = Vector4(z, w, -x, -y);
    return *this;
}

Quaternion& Quaternion::rotate180AroundWorldZ()
{
    elem = Vector4(-y, x, w, -z);
    return *this;
}

Quaternion& Quaternion::rotate180AroundLocalX()
{
    elem = Vector4(w, z, -y, -x);
    return *this;
}

Quaternion& Quaternion::rotate180AroundLocalY()
{
    elem = Vector4(-z, w, x, -y);
    return *this;
}

Quaternion& Quaternion::rotate180AroundLocalZ()
{
    elem = Vector4(y, -x, w, -z);
    return *this;
}

Quaternion Quaternion::slerp(Quaternion const& q1, Quaternion const& q2, float t)
{
    float cosAngle = q1.getNormalized().getImaginary().dot(q2.getNormalized().getImaginary());
    float angle = acosf(cosAngle);
    float sinAngle = sinf(angle);
    return q1 * (sinf((1 - t) * angle) / sinAngle) + q2 * (sinf(t * angle) / sinAngle);
}

Quaternion Quaternion::rotation(Vector3 axis, float radians)
{
    float angleRad = radians / 2.0f;
    Vector3 v = axis.getNormalized();
    float w = cosf(angleRad);
    v *= sinf(angleRad);
    return Quaternion(v, w);
}

Quaternion Quaternion::rotationX(float radians)
{
    return Quaternion::rotation(Vector3(1, 0, 0), radians);
}

Quaternion Quaternion::rotationY(float radians)
{
    return Quaternion::rotation(Vector3(0, 1, 0), radians);
}

Quaternion Quaternion::rotationZ(float radians)
{
    return Quaternion::rotation(Vector3(0, 0, 1), radians);
}
