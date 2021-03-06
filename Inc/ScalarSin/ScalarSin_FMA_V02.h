#pragma once

#include <DirectXMath.h>

using namespace DirectX;

constexpr uint32_t SIGN_BIT_MASK_FMA_V02 = 0x80000000;
constexpr XMVECTOR XM_HALF_FMA_V02 = { 0.5f, 0.5f, 0.5f, 0.5f };
constexpr XMVECTOR XM_PI_FMA_V02 = { XM_PI, XM_PI, XM_PI, XM_PI };
constexpr XMVECTOR XM_NEG_PI_FMA_V02 = { -XM_PI, -XM_PI, -XM_PI, -XM_PI };
constexpr XMVECTOR XM_2PI_FMA_V02 = { XM_2PI, XM_2PI, XM_2PI, XM_2PI };
constexpr XMVECTOR XM_1DIV2PI_FMA_V02 = { XM_1DIV2PI, XM_1DIV2PI, XM_1DIV2PI, XM_1DIV2PI };
constexpr XMVECTOR XM_PIDIV2_FMA_V02 = { XM_PIDIV2, XM_PIDIV2, XM_PIDIV2, XM_PIDIV2 };
constexpr XMVECTOR XM_NEG_PIDIV2_FMA_V02 = { -XM_PIDIV2, -XM_PIDIV2, -XM_PIDIV2, -XM_PIDIV2 };

constexpr XMVECTOR SIN_COEFF0_FMA_V02 = { -2.3889859e-08f, 0, 0, 0 };
constexpr XMVECTOR SIN_COEFF1_FMA_V02 = { 2.7525562e-06f, 0, 0, 0 };
constexpr XMVECTOR SIN_COEFF2_FMA_V02 = { 0.00019840874f, 0, 0, 0 };
constexpr XMVECTOR SIN_COEFF3_FMA_V02 = { 0.0083333310f, 0, 0, 0 };
constexpr XMVECTOR SIN_COEFF4_FMA_V02 = { 0.16666667f, 0, 0, 0 };
constexpr XMVECTOR SIN_COEFF5_FMA_V02 = { 1.0f, 0, 0, 0 };

// V23 tries to remove the second branch
__declspec(noinline) float XMScalarSin_FMA_V02(const float Value)
{
	const XMVECTOR value_v = _mm_set_ps1(Value);
	const XMVECTOR sign_bit_mask = _mm_broadcast_ss((float*)&SIGN_BIT_MASK_FMA_V02);

	XMVECTOR quotient = _mm_mul_ss(value_v, XM_1DIV2PI_FMA_V02);
	quotient = _mm_round_ss(quotient, quotient, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);

	XMVECTOR y = _mm_fnmadd_ss(quotient, XM_2PI_FMA_V02, value_v);
	XMVECTOR y_sign = _mm_and_ps(y, sign_bit_mask);
	XMVECTOR y_abs = _mm_andnot_ps(sign_bit_mask, y);
	XMVECTOR y_abs_le_halfpi = _mm_cmple_ss(y_abs, XM_PIDIV2_FMA_V02);
	// If |y| > pi/2, offset = y_sign * XM_PI, else offset = 0.0
	XMVECTOR offset = _mm_or_ps(y_sign, XM_PI_FMA_V02);
	// If |y| > pi/2, y = -(y + offset), else y = y
	y = _mm_blendv_ps(_mm_sub_ss(offset, y), y, y_abs_le_halfpi);

	// 11-degree minimax approximation
	XMVECTOR y2 = _mm_mul_ss(y, y);
	XMVECTOR result = _mm_fmadd_ss(y2, SIN_COEFF0_FMA_V02, SIN_COEFF1_FMA_V02);
	result = _mm_fmsub_ss(result, y2, SIN_COEFF2_FMA_V02);
	result = _mm_fmadd_ss(result, y2, SIN_COEFF3_FMA_V02);
	result = _mm_fmsub_ss(result, y2, SIN_COEFF4_FMA_V02);
	result = _mm_fmadd_ss(result, y2, SIN_COEFF5_FMA_V02);
	result = _mm_mul_ss(result, y);
	return _mm_cvtss_f32(result);
}
