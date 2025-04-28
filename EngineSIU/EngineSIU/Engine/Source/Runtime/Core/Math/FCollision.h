#pragma once

class UCapsuleComponent;
class USphereComponent;
class UBoxComponent;

struct FCollision
{
    static bool CheckOverlapBoxToBox(const UBoxComponent& Box1, const UBoxComponent& Box2);

    static bool CheckOverlapSphereToSphere(const USphereComponent& Sphere1, const USphereComponent& Sphere2);

    static bool CheckOverlapCapsuleToCapsule(const UCapsuleComponent& Capsule1, const UCapsuleComponent& Capsule2);

    static bool CheckOverlapBoxToSphere(const UBoxComponent& Box1, const USphereComponent& Sphere1);

    static bool CheckOverlapBoxToCapsule(const UBoxComponent& Box1, const UCapsuleComponent& Capsule1);

    static bool CheckOverlapSphereToCapsule(const USphereComponent& Sphere1, const UCapsuleComponent& Capsule1);
    
};
