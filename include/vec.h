#pragma once

#include <string.h>
#include <stdio.h>
#include <array>
#include <cmath>


namespace MathUtil {
template <unsigned size>
class Vec
{
public:
	Vec() = default;
	Vec(const float val) {
		for (int i = 0; i < size; i++) {
			_data[i] = val;
		}
	}
	Vec(const std::array<float, size>& arr) {
		memcpy(_data, arr.data(), arr.size() * sizeof(float));
	}

	const float& operator()(int index) const {
		return _data[index];
	}
	float& operator()(int index) {
		return _data[index];
	}

	~Vec() {}
	void Print() const {
		printf("[");
		for (int i = 0; i < size; i++) {
			printf("%14.5f", _data[i]);
		}
		printf("]\n");
	}
	unsigned Size() const {
		return size;
	}

	//Simple Arithmetic
	Vec<size> operator+ (const Vec<size>& u) const {
		Vec<size> result;
		for (int i = 0; i < size; i++) {
			result._data[i] = _data[i] + u._data[i];
		}
		return result;
	}
	Vec<size> operator-(const Vec& u) const {
		Vec<size> result;
		for (int i = 0; i < size; i++) {
			result._data[i] = _data[i] - u._data[i];
		}
		return result;
	}
	Vec<size> operator* (float scaler) const {
		Vec<size> result;
		for (int i = 0; i < size; i++) {
			result._data[i] = _data[i] * scaler;
		}
		return result;
	}
	Vec<size> operator/ (float scaler) const {
		Vec<size> result;
		for (int i = 0; i < size; i++) {
			result._data[i] = _data[i] / scaler;
		}
		return result;
	}
	void operator+= (const Vec<size>& u) {
		for (int i = 0; i < size; i++) {
			_data[i] += u._data[i];
		}
	}
	void operator-= (const Vec& u) {
		for (int i = 0; i < size; i++) {
			_data[i] -= u._data[i];
		}
	}
	void operator*= (float scaler) {
		for (int i = 0; i < size; i++) {
			_data[i] *= scaler;
		}
	}
	void operator/= (float scaler) {
		for (int i = 0; i < size; i++) {
			_data[i] /= scaler;
		}
	}

	//Special vector operations
	float Dot(const Vec<size>& u)const {
		float result = 0;
		for (int i = 0; i < size; i++) {
			result += _data[i] * u._data[i];
		}
		return result;
	}
	float LenSqr()const {
		return Dot(*this);
	}
	Vec<size> Proj(const Vec<size>& v)const {
		return v * (Dot(v) / v.LenSqr());
	}
	Vec<size> Normalized() const {
		Vec<size> result;
		float len = sqrt(LenSqr());
		for (int i = 0; i < size; i++) {
			result._data[i] =  _data[i]/len;
		}
		return result;
	}
	void Normalize() {
		float len = sqrt(LenSqr());
		for (int i = 0; i < size; i++) {
			_data[i] /= len;
		}
	}

	//description:
	//	output raw array of floats
	//Parameters:
	//	buffer: output buffer where matrix data is copied into, allocated externally
	void CopyRaw(float* buffer) const {
		memcpy(buffer, _data, sizeof(float) * size);
	}

	const void* GetData() const {
		return _data;
	}

	// clip all elements
	void Clip(const float& upper, const float& lower) {
		for (int i = 0; i < size; i++) {
			if (_data[i] > upper) _data[i] = upper;
			else if (_data[i < lower]) _data[i] = lower;
		}
	}

	float Sum() {
		float sum = 0;
		for (int i = 0; i < size; i++) sum += _data[i];
		return sum;
	}

private:
	float _data[size];
};

template<unsigned size>
Vec<size> operator* (float scaler, Vec<size> v) {
	return v * scaler;
}

inline Vec<3> Cross(const Vec<3>& v1, const Vec<3>& v2) {
	Vec<3> result;

	result(0) = v1(1)*v2(2)-v1(2)*v2(1);
	result(1) = v1(3)*v2(0)-v1(0)*v2(3);
	result(2) = v1(0)*v2(1)-v1(1)*v2(0);

	return result;
}

}