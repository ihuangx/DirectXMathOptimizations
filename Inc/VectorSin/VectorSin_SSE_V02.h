#pragma once

#include <DirectXMath.h>

using namespace DirectX;

constexpr float SIN_COEFF0_SSE_V02 = -2.3889859e-08f;
constexpr float SIN_COEFF1_SSE_V02 = 2.7525562e-06f;
constexpr float SIN_COEFF2_SSE_V02 = -0.00019840874f;
constexpr float SIN_COEFF3_SSE_V02 = 0.0083333310f;
constexpr float SIN_COEFF4_SSE_V02 = -0.16666667f;
constexpr float SIN_COEFF5_SSE_V02 = 1.0f;

// V02 loads the coefficients with broadcast
inline XMVECTOR XM_CALLCONV XMVectorSin_SSE_V02(FXMVECTOR V)
{
	// 11-degree minimax approximation
	// Force the value within the bounds of pi
	XMVECTOR quotient = _mm_mul_ps(V, g_XMReciprocalTwoPi);
	quotient = _mm_round_ps(quotient, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
	quotient = _mm_mul_ps(quotient, g_XMTwoPi);
	XMVECTOR x = _mm_sub_ps(V, quotient);

	// Map in [-pi/2,pi/2] with sin(y) = sin(x).
	XMVECTOR x_sign = _mm_and_ps(x, g_XMNegativeZero);
	XMVECTOR x_abs = _mm_andnot_ps(g_XMNegativeZero, x);
	XMVECTOR x_abs_le_halfpi = _mm_cmple_ps(x_abs, g_XMHalfPi);
	// If |y| > pi/2, offset = y_sign * XM_PI, else offset = 0.0
	XMVECTOR offset = _mm_or_ps(x_sign, g_XMPi);
	// If |y| > pi/2, y = -(y + offset), else y = y
	x = _mm_blendv_ps(_mm_sub_ps(offset, x), x, x_abs_le_halfpi);

	XMVECTOR x2 = _mm_mul_ps(x, x);

	// Compute polynomial approximation
	XMVECTOR Result = _mm_mul_ps(x2, _mm_broadcast_ss(&SIN_COEFF0_SSE_V02));
	Result = _mm_add_ps(Result, _mm_broadcast_ss(&SIN_COEFF1_SSE_V02));
	Result = _mm_mul_ps(Result, x2);
	Result = _mm_add_ps(Result, _mm_broadcast_ss(&SIN_COEFF2_SSE_V02));
	Result = _mm_mul_ps(Result, x2);
	Result = _mm_add_ps(Result, _mm_broadcast_ss(&SIN_COEFF3_SSE_V02));
	Result = _mm_mul_ps(Result, x2);
	Result = _mm_add_ps(Result, _mm_broadcast_ss(&SIN_COEFF4_SSE_V02));
	Result = _mm_mul_ps(Result, x2);
	Result = _mm_add_ps(Result, _mm_broadcast_ss(&SIN_COEFF5_SSE_V02));
	Result = _mm_mul_ps(Result, x);
	return Result;
}
