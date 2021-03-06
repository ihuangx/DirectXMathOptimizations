#pragma once

#include <DirectXMath.h>

using namespace DirectX;

// Align to a cache line
__declspec(align(64)) struct ScalarSinConstants_FMA_V03
{
	float pi;					//  4 bytes
	float neg_pi;				//  4 bytes
	float two_pi;				//  4 bytes
	float inv_two_pi;			//  4 bytes
	float half_pi;				//  4 bytes
	float neg_half_pi;			//  4 bytes

	float coefficients[6];		// 24 bytes

	// Total struct size:		   48 bytes
};

// Extern instead of constexpr since it forces the compiler to use the cache line
// aligned constants
extern ScalarSinConstants_FMA_V03 SCALAR_SIN_CONSTANTS_FMA_V03;

// V24 packs all the constants and inlines the function
// It inlines!
inline float XMScalarSin_FMA_V03(const float Value)
{
	const XMVECTOR value_v = _mm_set_ps1(Value);

	XMVECTOR quotient = _mm_mul_ss(value_v, *(XMVECTOR*)&SCALAR_SIN_CONSTANTS_FMA_V03.inv_two_pi);
	quotient = _mm_round_ss(quotient, quotient, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);

	XMVECTOR y = _mm_fnmadd_ss(quotient, *(XMVECTOR*)&SCALAR_SIN_CONSTANTS_FMA_V03.two_pi, value_v);
	if (_mm_comigt_ss(y, *(XMVECTOR*)&SCALAR_SIN_CONSTANTS_FMA_V03.half_pi))
	{
		y = _mm_sub_ss(*(XMVECTOR*)&SCALAR_SIN_CONSTANTS_FMA_V03.pi, y);	// 1 cmp, 1 sub, 3 mov, 2 jmp = 7 instructions
	}
	else if (!_mm_comile_ss(*(XMVECTOR*)&SCALAR_SIN_CONSTANTS_FMA_V03.neg_half_pi, y))
	{
		y = _mm_sub_ss(*(XMVECTOR*)&SCALAR_SIN_CONSTANTS_FMA_V03.neg_pi, y);	// 2 cmp, 1 sub, 3 mov, 2 jmp = 8 instructions
	}
	else
	{
		// 2 cmp, 2 jmp, 1 mov = 5 instructions
	}

	// 11-degree minimax approximation
	XMVECTOR y2 = _mm_mul_ss(y, y);
	XMVECTOR result = _mm_fmadd_ss(y2, *(XMVECTOR*)&SCALAR_SIN_CONSTANTS_FMA_V03.coefficients[0], *(XMVECTOR*)&SCALAR_SIN_CONSTANTS_FMA_V03.coefficients[1]);
	result = _mm_fmsub_ss(result, y2, *(XMVECTOR*)&SCALAR_SIN_CONSTANTS_FMA_V03.coefficients[2]);
	result = _mm_fmadd_ss(result, y2, *(XMVECTOR*)&SCALAR_SIN_CONSTANTS_FMA_V03.coefficients[3]);
	result = _mm_fmsub_ss(result, y2, *(XMVECTOR*)&SCALAR_SIN_CONSTANTS_FMA_V03.coefficients[4]);
	result = _mm_fmadd_ss(result, y2, *(XMVECTOR*)&SCALAR_SIN_CONSTANTS_FMA_V03.coefficients[5]);
	result = _mm_mul_ss(result, y);
	return _mm_cvtss_f32(result);
}
