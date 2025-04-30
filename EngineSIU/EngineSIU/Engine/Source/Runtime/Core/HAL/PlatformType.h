#pragma once
#include <cstdint>

//~ Windows.h
#define _TCHAR_DEFINED  // TCHAR 재정의 에러 때문
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#undef max
#undef min

#ifdef TEXT             // Windows.h의 TEXT를 삭제
    #undef TEXT
#endif
//~ Windows.h


// inline을 강제하는 매크로
#define FORCEINLINE __forceinline

// inline을 하지않는 매크로
#define FORCENOINLINE __declspec(noinline)

#ifdef _DEBUG
    #define FORCEINLINE_DEBUGGABLE inline
#else
    #define FORCEINLINE_DEBUGGABLE FORCEINLINE
#endif


#define USE_WIDECHAR 0

#if USE_WIDECHAR 
    #define TEXT(x) L##x
#else
    #define TEXT(x) x
#endif


// unsigned int type
typedef std::uint8_t uint8;
typedef std::uint16_t uint16;
typedef std::uint32_t uint32;
typedef std::uint64_t uint64;

// signed int
typedef std::int8_t int8;
typedef std::int16_t int16;
typedef std::int32_t int32;
typedef std::int64_t int64;

typedef char ANSICHAR;
typedef wchar_t WIDECHAR;

#if USE_WIDECHAR
typedef WIDECHAR TCHAR;
#else
typedef ANSICHAR TCHAR;
#endif

// 임시로 사용
#include <string>
using FWString = std::wstring;
