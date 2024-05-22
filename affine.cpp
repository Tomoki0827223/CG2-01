#include "affine.h"


Matrix4x4 MatrixMultipry(const Matrix4x4& matrix1, const Matrix4x4& matrix2) {
	Matrix4x4 result;

	for (int i = 0; i < 4; ++i) {

		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = matrix1.m[i][0] * matrix2.m[0][j] + matrix1.m[i][1] * matrix2.m[1][j] + matrix1.m[i][2] * matrix2.m[2][j] + matrix1.m[i][3] * matrix2.m[3][j];
		}
	}

	return result;
}

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	Matrix4x4 scaleMat = MakeScaleMatrix(scale);
	Matrix4x4 rotateMat = MakeRotateMatrix(rotate);
	Matrix4x4 translateMat = MakeTranslateMatrix(translate);

	return MatrixMultipry(MatrixMultipry(translateMat, rotateMat), scaleMat);
}

Matrix4x4 MakeScaleMatrix(const Vector3& scale) {
	Matrix4x4 result = {};

	result.m[0][0] = scale.x;
	result.m[1][1] = scale.y;
	result.m[2][2] = scale.z;
	result.m[3][3] = 1.0f;

	return result;
}

Matrix4x4 MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 result = {};

	result.m[0][0] = 1.0f;
	result.m[1][1] = 1.0f;
	result.m[2][2] = 1.0f;
	result.m[3][3] = 1.0f;
	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;

	return result;
}

Matrix4x4 MakeRotateMatrix(const Vector3& rotate) {
	Matrix4x4 rotateX = { 1, 0, 0, 0, 0, cosf(rotate.x), -sinf(rotate.x), 0, 0, sinf(rotate.x), cosf(rotate.x), 0, 0, 0, 0, 1 };

	Matrix4x4 rotateY = { cosf(rotate.y), 0, sinf(rotate.y), 0, 0, 1, 0, 0, -sinf(rotate.y), 0, cosf(rotate.y), 0, 0, 0, 0, 1 };

	Matrix4x4 rotateZ = { cosf(rotate.z), -sinf(rotate.z), 0, 0, sinf(rotate.z), cosf(rotate.z), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

	return MatrixMultipry(MatrixMultipry(rotateZ, rotateY), rotateX);
}
