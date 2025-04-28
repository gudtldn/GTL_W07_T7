#pragma once

class UObject;

/**
 * 주어진 UObject 포인터가 유효한지 확인합니다.
 *
 * @param Test 유효성을 검사할 UObject 포인터입니다.
 * @return 포인터가 유효하면 true를 반환하고, 그렇지 않으면 false를 반환합니다.
 */
bool IsValid(const UObject* Test);
