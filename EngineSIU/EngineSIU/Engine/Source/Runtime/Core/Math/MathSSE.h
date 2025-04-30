#pragma once
#include <immintrin.h>
#include "HAL/PlatformType.h"

/**
 * @param A0	Selects which element (0-3) from 'A' into 1st slot in the result
 * @param A1	Selects which element (0-3) from 'A' into 2nd slot in the result
 * @param B2	Selects which element (0-3) from 'B' into 3rd slot in the result
 * @param B3	Selects which element (0-3) from 'B' into 4th slot in the result
 */
#define SHUFFLEMASK(A0,A1,B2,B3) ( (A0) | ((A1)<<2) | ((B2)<<4) | ((B3)<<6) )

#define VectorReplicate(Vec, Index) VectorReplicateTemplate<Index>(Vec)

struct FMatrix;


// 4 floats
typedef __m128	VectorRegister4Float;


namespace SSE
{
/**
 * Vector의 특정 인덱스를 복제합니다.
 * @tparam Index 복제할 Index (0 ~ 3)
 * @param Vector 복제할 대상
 * @return 복제된 레지스터
 */
template <int Index>
FORCEINLINE VectorRegister4Float VectorReplicateTemplate(const VectorRegister4Float& Vector)
{
    return _mm_shuffle_ps(Vector, Vector, SHUFFLEMASK(Index, Index, Index, Index));
}


/** Vector4 곱연산 */
FORCEINLINE VectorRegister4Float VectorMultiply(const VectorRegister4Float& Vec1, const VectorRegister4Float& Vec2)
{
    return _mm_mul_ps(Vec1, Vec2);
}

FORCEINLINE VectorRegister4Float VectorAdd(const VectorRegister4Float& Vec1, const VectorRegister4Float& Vec2)
{
    return _mm_add_ps(Vec1, Vec2);
}

FORCEINLINE VectorRegister4Float VectorMultiplyAdd(
    const VectorRegister4Float& Vec1,
    const VectorRegister4Float& Vec2,
    const VectorRegister4Float& Vec3
)
{
    return VectorAdd(VectorMultiply(Vec1, Vec2), Vec3);
}

inline void VectorMatrixMultiply(FMatrix* Result, const FMatrix* Matrix1, const FMatrix* Matrix2)
{
    // 레지스터에 값 로드
    const VectorRegister4Float* Matrix1Ptr = reinterpret_cast<const VectorRegister4Float*>(Matrix1);
    const VectorRegister4Float* Matrix2Ptr = reinterpret_cast<const VectorRegister4Float*>(Matrix2);
    VectorRegister4Float* Ret = reinterpret_cast<VectorRegister4Float*>(Result);
    VectorRegister4Float Temp, R0, R1, R2;

    // 첫번째 행 계산
    Temp = VectorMultiply(VectorReplicate(Matrix1Ptr[0], 0), Matrix2Ptr[0]);
    Temp = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[0], 1), Matrix2Ptr[1], Temp);
    Temp = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[0], 2), Matrix2Ptr[2], Temp);
    R0 = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[0], 3), Matrix2Ptr[3], Temp);

    // 두번째 행 계산
    Temp = VectorMultiply(VectorReplicate(Matrix1Ptr[1], 0), Matrix2Ptr[0]);
    Temp = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[1], 1), Matrix2Ptr[1], Temp);
    Temp = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[1], 2), Matrix2Ptr[2], Temp);
    R1 = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[1], 3), Matrix2Ptr[3], Temp);

    // 세번째 행 계산
    Temp = VectorMultiply(VectorReplicate(Matrix1Ptr[2], 0), Matrix2Ptr[0]);
    Temp = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[2], 1), Matrix2Ptr[1], Temp);
    Temp = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[2], 2), Matrix2Ptr[2], Temp);
    R2 = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[2], 3), Matrix2Ptr[3], Temp);

    // 네번째 행 계산
    Temp = VectorMultiply(VectorReplicate(Matrix1Ptr[3], 0), Matrix2Ptr[0]);
    Temp = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[3], 1), Matrix2Ptr[1], Temp);
    Temp = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[3], 2), Matrix2Ptr[2], Temp);
    Temp = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[3], 3), Matrix2Ptr[3], Temp);

    // 결과 저장
    Ret[0] = R0;
    Ret[1] = R1;
    Ret[2] = R2;
    Ret[3] = Temp;
}

FORCEINLINE float TruncToFloat(float F)
{
    return _mm_cvtss_f32(_mm_round_ps(_mm_set_ss(F), 3));
}

FORCEINLINE double TruncToDouble(double F)
{
    return _mm_cvtsd_f64(_mm_round_pd(_mm_set_sd(F), 3));
}

FORCEINLINE float FloorToFloat(float F)
{
    return _mm_cvtss_f32(_mm_floor_ps(_mm_set_ss(F)));
}

FORCEINLINE double FloorToDouble(double F)
{
    return _mm_cvtsd_f64(_mm_floor_pd(_mm_set_sd(F)));
}

FORCEINLINE float RoundToFloat(float F)
{
    return FloorToFloat(F + 0.5f);
}

FORCEINLINE double RoundToDouble(double F)
{
    return FloorToDouble(F + 0.5);
}

FORCEINLINE float CeilToFloat(float F)
{
    return _mm_cvtss_f32(_mm_ceil_ps(_mm_set_ss(F)));
}

FORCEINLINE double CeilToDouble(double F)
{
    return _mm_cvtsd_f64(_mm_ceil_pd(_mm_set_sd(F)));
}
}
