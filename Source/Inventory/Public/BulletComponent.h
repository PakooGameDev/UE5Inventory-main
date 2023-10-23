// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h" 
#include <Kismet/KismetMathLibrary.h>
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Engine/DataTable.h"
#include "DrawDebugHelpers.h"
#include "Components/DecalComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"  
#include "BulletComponent.generated.h"


USTRUCT(BlueprintType)
struct FPenetrationData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float Density;
};

USTRUCT(BlueprintType)
struct FBulletParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BulletParams")
		float Caliber;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BulletParams")
		float Length;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BulletParams")
		float Mass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BulletParams")
		float Density;

	// Конструктор по умолчанию
	FBulletParameters()
		: Caliber(0.0f), Length(0.0f), Mass(0.0f), Density(0.0f)
	{
	}

	FBulletParameters(float InCaliber, float InLength, float InMass, float InDensity)
		: Caliber(InCaliber), Length(InLength), Mass(InMass), Density(InDensity)
	{	
	}
};



class UPhysicalMaterial;
class UMaterialInterface;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INVENTORY_API UBulletComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBulletComponent();

	AActor* Owner;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enviroment")
		UDataTable* MyDataTable;

	FCollisionQueryParams CollisionParams;


private:


	//Debug
	UPROPERTY(EditAnywhere, Category = "Debug")
		bool IsDebugLineVisible = false;
	UPROPERTY(EditAnywhere, Category = "Debug")
		bool IsDebugSphereVisible = false;
	
	//Bullet
	UPROPERTY(EditAnywhere, Category = "BulletParams")
		float LifeTime = 3.f;
	UPROPERTY(EditAnywhere, Category = "BulletParams")
		float BulletSpeed = 75000.f;
	UPROPERTY(EditAnywhere, Category = "BulletParams")
		FBulletParameters BulletParameters = FBulletParameters(7.62f, 39.f, 12.6f, 7.83f);

	//BulletHole
	UPROPERTY(EditAnywhere, Category = "BulletHoleDecal")
		UMaterialInterface* DecalMaterial;
	UPROPERTY(EditAnywhere, Category = "BulletHoleDecal")
		FVector DecalSize = FVector(0.1f, 20.f, 20.f);
	UPROPERTY(EditAnywhere, Category = "BulletHoleDecal")
		float DecalLifeSpan = 20.f;
	UPROPERTY(EditAnywhere, Category = "BulletHoleDecal")
		float DecalFadeScreenSize = 0.001f;;

	//Enviroment
	UPROPERTY(EditAnywhere, Category = "Enviroment")
		float AirDensity = 0.001225f;
	UPROPERTY(EditAnywhere, Category = "Enviroment")
		float Gravity = 981.f;
	UPROPERTY(EditAnywhere, Category = "Enviroment")
		float DragCoefficincy = 0.47f;


	FTimerHandle TimerHandle_DestroyActor;


	AActor* ActorToIgnore;

	FVector Velocity;

	float BulletDeltaTime = 0.0f;

	float PenetrationDepth = 0.0f;

	FVector FirstPos;

	FVector LastPos;

	bool isInitialized = false;

	bool Penetrated = false;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


private:

	UFUNCTION()
		float AngleBetweenTwoVectors(const FVector& Vector1, const FVector& Vector2);

	UFUNCTION()
		float CalculatePenetrationDepth(float MaterialDensity);

	UFUNCTION()
		FLinearColor GetTraceColor();

	UFUNCTION()
		bool WhetherPenetrate(FHitResult Hit);

	UFUNCTION()
		void DestroyOwnerActor();

	UFUNCTION()
		void BuildTrajectory();

	UFUNCTION()
		void ComputeExitLocation(FHitResult Hit, float Depth);

	UFUNCTION()
		void CalculateAcceleration();

	UFUNCTION()
		void VaryTrajectory(FVector SurfaceNormal);

	UFUNCTION()
		void DrawLine(FHitResult Hit);

	UFUNCTION()
		void RecursiveTrace(FHitResult Hit, FVector& Start, FVector End, TArray<AActor*>& InIgnoreActors);
	
	UFUNCTION()
		void SpawnHole(FHitResult HitResult);


};
