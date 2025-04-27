#pragma once
#include "Container/Array.h"

class UObject;
class UClass;

/**
 * ClassToLookFor와 일치하는 UObject를 반환합니다.
 * @param ClassToLookFor 반환할 Object의 Class정보
 * @param Results ClassToLookFor와 일치하는 모든 Objects가 담길 목록
 * @param bIncludeDerivedClasses ClassToLookFor의 파생 클래스까지 찾을지 여부
 */
void GetObjectsOfClass(const UClass* ClassToLookFor, TArray<UObject*>& Results, bool bIncludeDerivedClasses);

/**
 * ClassToChildListMap에 상속 구조를 저장합니다.
 * @note UClass에서 자동으로 처리할 때 사용되며, 직접 사용해서는 안됩니다.
 */
void AddClassToChildListMap(UClass* InClass);

/** FUObjectHashTables에 Object의 정보를 저장합니다. */
void AddToClassMap(UObject* Object);

/** FUObjectHashTables에 저장된 Object정보를 제거합니다. */
void RemoveFromClassMap(UObject* Object);

/**
 * ClassToLookFor와 일치하는 자식 UClass를 반환합니다.
 * @param ClassToLookFor 찾을 자식클래스의 부모 클래스
 * @param Results ClassToLookFor의 파생클래스가 담길 목록
 * TODO: GetClassMap 사용해서 Spawn 안 된 UClass도 찾기
 */
void GetChildOfClass(UClass* ClassToLookFor, TArray<UClass*>& Results);

/** ClassToLookFor와 일치하는 오브젝트의 개수를 반환합니다. */
uint32 GetNumOfObjectsByClass(UClass* ClassToLookFor);
