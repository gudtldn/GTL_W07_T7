#include "String.h"
#include <algorithm>
#include <vector>

#include "CoreMiscDefines.h"
#include "Math/MathUtility.h"


FString FString::SanitizeFloat(float InFloat)
{
#if USE_WIDECHAR
    return FString{std::to_wstring(InFloat)};
#else
    return FString{std::to_string(InFloat)};
#endif
}

float FString::ToFloat(const FString& InString)
{
	return std::stof(*InString);
}

int FString::ToInt(const FString& InString)
{
    return std::stoi(*InString);
}

bool FString::ToBool() const
{
    // 빈 문자열은 false로 처리
    if (IsEmpty())
    {
        return false;
    }

    // 가장 일반적인 경우: "true" 또는 "1" (대소문자 무관)
    // Equals 함수가 이미 대소문자 무시 비교를 지원하므로 활용합니다.
    if (Equals(TEXT("true"), ESearchCase::IgnoreCase))
    {
        return true;
    }
    if (Equals(TEXT("1"))) // "1"은 대소문자 구분이 의미 없음
    {
        return true;
    }

    // 그 외: "false" 또는 "0" (대소문자 무관)
    // 이 경우들도 명시적으로 false를 반환하는 것이 안전합니다.
    if (Equals(TEXT("false"), ESearchCase::IgnoreCase))
    {
        return false;
    }
    if (Equals(TEXT("0"))) // "0"도 대소문자 구분이 의미 없음
    {
        return false;
    }

    // 위 조건에 해당하지 않는 모든 다른 문자열은 false로 처리합니다.
    // (예: "Yes", "No", "On", "Off" 등을 추가로 지원하고 싶다면 여기에 조건을 추가할 수 있습니다.)
    // UE_LOG(LogTemp, Warning, TEXT("FString::ToBool() : Unrecognized string '%s' treated as false."), **this); // 필요시 경고 로그
    return false;
}

FString FString::RightChop(int32 Count) const
{
    const int32 MyLen = Len(); // 현재 문자열 길이

    // Count가 0 이하이면 원본 문자열의 복사본을 반환
    if (Count <= 0)
    {
        return *this; // 복사본 반환
    }

    // Count가 문자열 길이 이상이면 빈 문자열 반환
    if (Count >= MyLen)
    {
        return FString{}; // 기본 생성된 빈 FString 반환
    }

    // std::basic_string::substr(pos)는 위치 pos부터 끝까지의 부분 문자열을 반환합니다.
    // Count는 제거할 문자의 개수이므로, 부분 문자열은 Count 인덱스부터 시작합니다.
    // static_cast<size_t>는 substr이 size_t를 인자로 받기 때문에 필요합니다.
    BaseStringType Substring = PrivateString.substr(Count);

    // 추출된 부분 문자열로 새로운 FString 객체를 생성하여 반환
    // std::move를 사용하면 불필요한 복사를 피할 수 있습니다 (C++11 이상).
    return FString{std::move(Substring)};
}

void FString::Empty()
{
    PrivateString.clear();
}

bool FString::Equals(const FString& Other, ESearchCase::Type SearchCase) const
{
    const int32 Num = Len();
    const int32 OtherNum = Other.Len();

    if (Num != OtherNum)
    {
        // Handle special case where FString() == FString("")
        return Num + OtherNum == 1;
    }
    else if (Num > 1)
    {
        if (SearchCase == ESearchCase::CaseSensitive)
        {
        	return TCString<ElementType>::Strcmp(**this, *Other) == 0;
        }
        else
        {
        	return std::ranges::equal(
		        PrivateString, Other.PrivateString, [](char a, char b)
	        {
		        return std::tolower(a) == std::tolower(b);
	        });
        }
    }

    return true;
}

bool FString::Contains(const FString& SubStr, ESearchCase::Type SearchCase, ESearchDir::Type SearchDir) const
{
    return Find(SubStr, SearchCase, SearchDir, 0) != INDEX_NONE;
}

int32 FString::Find(
    const FString& SubStr, ESearchCase::Type SearchCase, ESearchDir::Type SearchDir, int32 StartPosition
) const
{
    if (SubStr.IsEmpty() || IsEmpty())
    {
        return INDEX_NONE;
    }

    const ElementType* StrPtr = **this;
    const ElementType* SubStrPtr = *SubStr;
    const int32 StrLen = Len();
    const int32 SubStrLen = SubStr.Len();

    auto CompareFunc = [SearchCase](ElementType A, ElementType B) -> bool {
        return (SearchCase == ESearchCase::IgnoreCase) ? 
            tolower(A) == tolower(B) : A == B;
    };

    auto FindSubString = [&](int32 Start, int32 End, int32 Step) -> int32 {
        for (int32 i = Start; i != End; i += Step)
        {
            bool Found = true;
            for (int32 j = 0; j < SubStrLen; ++j)
            {
                if (!CompareFunc(StrPtr[i + j], SubStrPtr[j]))
                {
                    Found = false;
                    break;
                }
            }
            if (Found)
            {
                return i;
            }
        }
        return INDEX_NONE;
    };

    if (SearchDir == ESearchDir::FromStart)
    {
        StartPosition = FMath::Clamp(StartPosition, 0, StrLen - SubStrLen);
        return FindSubString(StartPosition, StrLen - SubStrLen + 1, 1);
    }
    else // ESearchDir::FromEnd
    {
        StartPosition = (StartPosition == INDEX_NONE) ? StrLen - SubStrLen : FMath::Min(StartPosition, StrLen - SubStrLen);
        return FindSubString(StartPosition, -1, -1);
    }
}

void FString::Reserve(int32 CharacterCount)
{
    PrivateString.reserve(CharacterCount);
}

void FString::Resize(int32 CharacterCount)
{
    PrivateString.resize(CharacterCount);
}

FString FString::ToUpper() const &
{
    BaseStringType UpperCaseString = PrivateString;
    std::ranges::transform(
        UpperCaseString,
        UpperCaseString.begin(),
        [](ElementType Char) { return std::toupper(Char); }
    );
    return FString{std::move(UpperCaseString)};
}

FString FString::ToUpper() &&
{
    std::ranges::transform(
        PrivateString,
        PrivateString.begin(),
        [](ElementType Char) { return std::toupper(Char); }
    );
    return std::move(*this);
}

void FString::ToUpperInline()
{
    std::ranges::transform(
        PrivateString,
        PrivateString.begin(),
        [](ElementType Char) { return std::toupper(Char); }
    );
}

FString FString::ToLower() const &
{
    BaseStringType LowerCaseString = PrivateString;
    std::ranges::transform(
        LowerCaseString,
        LowerCaseString.begin(),
        [](ElementType Char) { return std::tolower(Char); }
    );
    return FString{std::move(LowerCaseString)};
}

FString FString::ToLower() &&
{
    std::ranges::transform(
        PrivateString,
        PrivateString.begin(),
        [](ElementType Char) { return std::tolower(Char); }
    );
    return std::move(*this);
}

void FString::ToLowerInline()
{
    std::ranges::transform(
        PrivateString,
        PrivateString.begin(),
        [](ElementType Char) { return std::tolower(Char); }
    );
}


// Printf 함수 구현
FString FString::Printf(const ElementType* Format, ...)
{
    if (!Format) // 포맷 문자열 null 체크
    {
        return FString{};
    }

    // 첫 번째 시도: 스택에 작은 버퍼를 할당 (일반적인 경우를 빠르게 처리)
    ElementType StaticBuffer[512];
    va_list ArgPtr;
    va_start(ArgPtr, Format);

    int32 Result = -1;
#if USE_WIDECHAR
    #ifdef _WIN32
        // _vsnwprintf는 널 종료를 보장하지 않을 수 있으며, 성공 시 문자 수(널 제외) 또는 버퍼가 작으면 -1 반환
        Result = _vsnwprintf(StaticBuffer, sizeof(StaticBuffer) / sizeof(ElementType), Format, ArgPtr);
    #else
        // vswprintf는 C99 표준부터 버퍼 크기를 받고 널 종료를 보장. 성공 시 문자 수(널 제외), 오류 시 음수 반환.
        Result = vswprintf(StaticBuffer, sizeof(StaticBuffer) / sizeof(ElementType), Format, ArgPtr);
    #endif
#else
    // vsnprintf는 C99 표준부터 버퍼 크기를 받고 널 종료를 보장. 성공 시 문자 수(널 제외), 오류 시 음수 반환.
    Result = vsnprintf(StaticBuffer, sizeof(StaticBuffer) / sizeof(ElementType), Format, ArgPtr);
#endif
    va_end(ArgPtr);

    // 작은 버퍼로 충분했고 오류가 없었다면 바로 반환
    if (Result >= 0 && Result < static_cast<int32>(sizeof(StaticBuffer) / sizeof(ElementType)))
    {
        // StaticBuffer[Result] = 0; // _vsnwprintf는 널 종료 보장 안할 수 있으나, vsnprintf/vswprintf C99는 함. 안전하게 추가 가능.
        return FString(BaseStringType(StaticBuffer));
    }
    else // 버퍼가 너무 작거나 오류 발생
    {
        // 두 번째 시도: 필요한 크기를 계산하여 동적 할당
        int32 RequiredSize = -1;
        va_list ArgPtr2;
        va_start(ArgPtr2, Format);
#if USE_WIDECHAR
    #ifdef _WIN32
        // _vsnwprintf에 null 버퍼와 0 크기를 전달하면 필요한 크기(널 포함 안함) 반환
        RequiredSize = _vsnwprintf(nullptr, 0, Format, ArgPtr2);
    #else
        // C99 vswprintf 동작은 구현에 따라 다를 수 있음. 일반적으로 비슷하게 동작.
        RequiredSize = vswprintf(nullptr, 0, Format, ArgPtr2); // 이 방식이 표준은 아닐 수 있음. 대안 필요시 다른 라이브러리 사용.
        // 임시 버퍼를 크게 잡고 시도하는 방법도 있음.
        // 또는 C++20 std::format 사용 고려.
    #endif
#else
        // C99 vsnprintf에 null 버퍼와 0 크기를 전달하면 필요한 크기(널 포함 안함) 반환
        RequiredSize = vsnprintf(nullptr, 0, Format, ArgPtr2);
#endif
        va_end(ArgPtr2);

        if (RequiredSize < 0) // 크기 계산 실패 (오류)
        {
            // 오류 로그 출력 가능
            return FString{}; // 빈 문자열 반환
        }

        // 필요한 크기 + 널 종료 문자 공간 할당
        std::vector<ElementType> DynamicBuffer(RequiredSize + 1);

        // 다시 포맷팅 수행
        va_list ArgPtr3;
        va_start(ArgPtr3, Format);
#if USE_WIDECHAR
    #ifdef _WIN32
        Result = _vsnwprintf(DynamicBuffer.data(), DynamicBuffer.size(), Format, ArgPtr3);
    #else
        Result = vswprintf(DynamicBuffer.data(), DynamicBuffer.size(), Format, ArgPtr3);
    #endif
#else
        Result = vsnprintf(DynamicBuffer.data(), DynamicBuffer.size(), Format, ArgPtr3);
#endif
        va_end(ArgPtr3);

        if (Result >= 0 && Result < static_cast<int32>(DynamicBuffer.size()))
        {
            // DynamicBuffer[Result] = 0; // 널 종료 보장됨 (C99)
            return FString(BaseStringType(DynamicBuffer.data()));
        }
        else
        {
            // 최종 포맷팅 실패 (이론상 발생하기 어려움)
            // 오류 로그 출력 가능
            return FString{}; // 빈 문자열 반환
        }
    }
}
