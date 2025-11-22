#pragma once
#include <array>
#include "vec.h"


namespace MathUtil {

template <unsigned rows, unsigned columns>
class Matrix
{
public:
	Matrix() = default;
	Matrix(float value) { for (int i = 0; i < (rows * columns); i++) _data[i] = value; }
	Matrix(const std::array<float, rows* columns>& values) { 
		for(int i = 0; i < rows; i++) {
			for(int j = 0; j < columns; j++) {
				_data[rows * j + i] = values[columns * i + j];
			}
		}
	}
	
	~Matrix() {}
	unsigned GetRows() const { return rows; }
	unsigned GetColumns() const { return columns; }

	const float& operator() (unsigned row, unsigned column) const {
		return _data[rows * column + row];
	}
	float& operator() (unsigned row, unsigned column) {
		return _data[rows * column + row];
	}

	Matrix<rows,columns> operator+(const Matrix< rows, columns>& second) const {
		Matrix<rows, columns> result;
		for (int i = 0; i < rows * columns; i++) {
			result._data[i] = _data[i] + second._data[i];
		}
		return result;
	}
	Matrix<rows, columns> operator-(const Matrix< rows, columns>& second) const {
		Matrix<rows, columns> result;
		for (int i = 0; i < rows * columns; i++) {
			result._data[i] = _data[i] - second._data[i];
		}
		return result;
	}
	template<unsigned rhsColumns>
	Matrix<rows, rhsColumns> operator*(const Matrix<columns, rhsColumns>& second) const {
		Matrix<rows, rhsColumns> result;

		for (unsigned i = 0; i < rows; i++) {
			for (unsigned j = 0; j < rhsColumns; j++) {
				result._data[j * rows + i] = 0.0f;
				for (unsigned k = 0; k < columns; k++) {
					result._data[j * rows + i] += _data[k * rows + i] * second._data[j * columns + k];
				}
			}
		}
		return result;
	}
	
	Vec<rows> operator*(const Vec<columns>& v) const {
		Vec<rows> result(0);
		for (unsigned i = 0; i < rows; i++) {
			for (unsigned k = 0; k < columns; k++) {
				result(i) += _data[k * rows + i] * v(k);
			}
		}
		return result;
	}

	Matrix<rows, columns> operator*(float c) const {
		Matrix<rows, columns> result;
		for (int i = 0; i < rows * columns; i++) {
			result._data[i] = _data[i] * c;
		}
		return result;
	}
	Matrix<rows, columns> operator/(float c) const {
		Matrix<rows, columns> result;
		for (int i = 0; i < rows * columns; i++) {
			result._data[i] = _data[i] / c;
		}
		return result;
	}
	
	void operator+=(const Matrix<rows,columns>& second) {
		for (int i = 0; i < rows * columns; i++) {
			_data[i] += second._data[i];
		}
	}
	void operator-=(const Matrix& second) {
		for (int i = 0; i < rows * columns; i++) {
			_data[i] -= second._data[i];
		}
	}
	void operator*=(float c) {
		for (int i = 0; i < rows * columns; i++) {
			_data[i] *= c;
		}
	}
	void operator/=(float c) {
		for (int i = 0; i < rows * columns; i++) {
			_data[i] /= c;
		}
	}

	//description:
	//	get a flattened array of all floats from the Matrix by stacking each row together
	//Parameters:
	//	buffer: output buffer where matrix data is copied into, allocated externally
	void CopyRaw(float* buffer) const {
		memcpy(buffer, _data, sizeof(float) * columns * rows);
	}

	void Print() const {
		printf("data addr: 0x%0x\n", _data);
		for (unsigned i = 0; i < rows; i++) {
			printf("[");
			for (unsigned j = 0; j < columns; j++) {
				printf("%14.5f" ,_data[j * rows + i]);
			}
			printf("]\n");
		}
		printf("\n");
	}

private:
	float _data[rows * columns];

	template <unsigned r, unsigned c> friend class Matrix;
};

template<unsigned dim>
Matrix<dim, dim> GetIdentity() {
	Matrix<dim, dim> result(0);
	for (int i = 0; i < dim; i++) result(i, i) = 1.0f;
	
	return result;
}

template<unsigned rows, unsigned columns>
Matrix<rows, columns> operator*(float c, const Matrix<rows, columns>& mat) {
	return (mat * c);
}
}