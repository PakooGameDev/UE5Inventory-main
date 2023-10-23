// Fill out your copyright notice in the Description page of Project Settings.

#include "BulletComponent.h"




// Sets default values for this component's properties
UBulletComponent::UBulletComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.01f;
    
    CollisionParams.bReturnPhysicalMaterial = true;

    static ConstructorHelpers::FObjectFinder<UDataTable> MyDataTableObj(TEXT("DataTable'/Game/GameProject/Blueprints/Ballistics/Data.Data'"));
    if (MyDataTableObj.Succeeded())
    {
        MyDataTable = MyDataTableObj.Object;
    }
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MyDecalMaterial(TEXT("Material'/Game/GameProject/Blueprints/Ballistics/M_BulletHoleDecal.M_BulletHoleDecal'"));
    if (MyDecalMaterial.Succeeded())
    {
        DecalMaterial = MyDecalMaterial.Object;
    }
}


// Called when the game starts
void UBulletComponent::BeginPlay()
{
	Super::BeginPlay();


    Owner = GetOwner();
    Velocity = FVector(Owner->GetActorForwardVector() * BulletSpeed);
    LastPos = Owner->GetActorLocation();
    isInitialized = true;

    // Delay the destruction of the owner actor by 5 seconds
    GetWorld()->GetTimerManager().SetTimer(TimerHandle_DestroyActor, this, &UBulletComponent::DestroyOwnerActor, LifeTime, false);
}

// Called every frame
void UBulletComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (isInitialized) {
        BulletDeltaTime += DeltaTime;
        BuildTrajectory();
    }

    if (Owner->GetActorLocation().Z < 0.0f) {
        Owner->Destroy();
    }
}



void UBulletComponent::BuildTrajectory()
{ 
    FHitResult HitResult;
    FirstPos = LastPos;
    LastPos = FirstPos + Velocity * BulletDeltaTime;
    if (IsDebugSphereVisible) {
        DrawDebugSphere(GetWorld(), LastPos, 20, 4, FColor::Red, false, 5.f);
    }
    if (GetWorld()->LineTraceSingleByChannel(HitResult, FirstPos, LastPos, ECC_Visibility, CollisionParams))
    {
        DrawLine(HitResult);
        if (WhetherPenetrate(HitResult))
        {
            SpawnHole(HitResult);
            ComputeExitLocation(HitResult, PenetrationDepth);
            if (Penetrated) {
                ActorToIgnore = HitResult.GetActor();
                CollisionParams.AddIgnoredActor(ActorToIgnore);

                VaryTrajectory(HitResult.ImpactNormal * (-1.f));
            }
            else {
                Owner->Destroy();
            }
        }
        else {
            //..
        }
    }
    else {
        DrawLine(HitResult);
        Owner->SetActorLocation(LastPos);
        CalculateAcceleration();
    }
}

bool UBulletComponent::WhetherPenetrate(FHitResult Hit)
{
    if (MyDataTable)
    {
        FName MatName = Hit.PhysMaterial.Get()->GetFName();

       // GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("%s"), *MatName.ToString()));
        FPenetrationData* DataRow = MyDataTable->FindRow<FPenetrationData>(FName(MatName), TEXT(""));

        if (DataRow)
        {
            PenetrationDepth = CalculatePenetrationDepth(DataRow->Density);

           // GEngine->AddOnScreenDebugMessage(-1, 200, FColor::Yellow, FString::Printf(TEXT("%f"), PenetrationDepth));

            if (PenetrationDepth > 0.0f)
            {
                return true;
            }
        }
    }
    return false;

}

void UBulletComponent::ComputeExitLocation(FHitResult Hit, float Depth)
{
    FVector Direction = FVector(Velocity.GetSafeNormal());

    FVector orgin;
    FVector boundsExtent;

    Hit.GetActor()->GetActorBounds(false, orgin, boundsExtent);
    
    float TraceDistance = FMath::Max3(boundsExtent.X, boundsExtent.Y, boundsExtent.Z) * 2.f;

    FVector TraceStart = FVector(Hit.ImpactPoint + (Direction * TraceDistance));
    FVector TraceEnd = FVector(TraceStart + (Direction * TraceDistance * (-1.0f) ));

    TArray<AActor*> InIgnoreActors;

    RecursiveTrace(Hit, TraceStart, TraceEnd, InIgnoreActors);
}

void UBulletComponent::RecursiveTrace(FHitResult Hit, FVector& Start, FVector End, TArray<AActor*>& InIgnoreActors)
{
    FHitResult HitResult;
    FCollisionQueryParams RecursiveCollisionParams;
    RecursiveCollisionParams.AddIgnoredActors(InIgnoreActors);

    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, RecursiveCollisionParams)) {
        if (HitResult.GetActor() != Hit.GetActor()) 
        {
            InIgnoreActors.Add(HitResult.GetActor());
            Start = HitResult.ImpactPoint;
            RecursiveTrace(Hit, Start, End, InIgnoreActors);
        }
        else {
            float tickness = FVector::Dist(HitResult.ImpactPoint, Hit.ImpactPoint);

            if (PenetrationDepth > tickness) {
                Velocity *= (1.f - tickness * 0.01f);
                LastPos = HitResult.ImpactPoint;
                Owner->SetActorLocation(LastPos);
                CalculateAcceleration();
                SpawnHole(HitResult);
                Penetrated = true;
            }
            else {
                Penetrated = false;
                Velocity = FVector(0, 0, 0);
            }
        }
    }
}

void UBulletComponent::CalculateAcceleration()
{
    FVector GravityForce = FVector(0.f, 0.f, (-Gravity * (BulletParameters.Mass / 1000.f)));
    FVector DragForce = (0.5f * AirDensity * Velocity.SizeSquared() * DragCoefficincy * (FMath::Square((BulletParameters.Caliber/2.f/10.f))*PI)) * (-Velocity.GetSafeNormal());

    Velocity += (GravityForce + DragForce) * BulletDeltaTime;
}   

void UBulletComponent::VaryTrajectory(FVector SurfaceNormal)
{
    FVector InitialAnswer = FVector(Velocity.Size() * UKismetMathLibrary::RandomUnitVectorInConeInRadians(Velocity, FMath::GetMappedRangeValueClamped(FVector2D(10000.f, 10000.f), FVector2D(0.087266f, 0.174533f), Velocity.Size())));
    float angle = AngleBetweenTwoVectors(SurfaceNormal, InitialAnswer);
    if (angle >= 90)
    {
        FRotator Rotator = FRotator((1.f - angle / 90) * UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::MakeRotFromX(SurfaceNormal), UKismetMathLibrary::MakeRotFromX(InitialAnswer)));
        Velocity = Rotator.RotateVector(InitialAnswer);
    }
    else {
        Velocity = InitialAnswer;
    }
}

float UBulletComponent::AngleBetweenTwoVectors(const FVector& Vector1, const FVector& Vector2)
{
    // Используйте функцию Dot Product для нахождения косинуса угла между векторами
    float CosineAngle = FVector::DotProduct(Vector1.GetSafeNormal(), Vector2.GetSafeNormal());

    // Используйте функцию ACos для нахождения угла в радианах и преобразуйте его в градусы
    float AngleInRadians = FMath::Acos(CosineAngle);
    float AngleInDegrees = FMath::RadiansToDegrees(AngleInRadians);

    return AngleInDegrees;
}

void UBulletComponent::DestroyOwnerActor()
{
    if (Owner)
    {
        Owner->Destroy();
    }
}

float UBulletComponent::CalculatePenetrationDepth(float MaterialDensity)
{
  
    return (BulletParameters.Length / 10.f) * (BulletParameters.Density / MaterialDensity);
}

void UBulletComponent::DrawLine(FHitResult Hit)
{
    if (GetWorld() && IsDebugLineVisible)
    {
        
        FColor LineColor = GetTraceColor().ToFColor(true);
        FVector End = Hit.bBlockingHit ? Hit.ImpactPoint : LastPos;
        // Draw the debug line
        DrawDebugLine(
            GetWorld(),
            FirstPos,
            End,
            LineColor,
            false, // Persistent (false means the line will disappear after LineDuration)
            5.0f
        );
    }
}

FLinearColor UBulletComponent::GetTraceColor()
{
    float coefficient = Velocity.Size() / BulletSpeed;
    if (coefficient > 0.75f) {
        float alpha = (coefficient - 0.75f);
        return FLinearColor(FMath::Lerp(FLinearColor(0.223228f, 0.0f, 0.001518f, 1.0f), FLinearColor(0.005208f, 0.000029f, 0.0f, 1.0f), alpha));
    }
    if (coefficient > 0.5f) {
        float alpha = (coefficient - 0.5f);
        return FLinearColor(FMath::Lerp(FLinearColor(1.0f, 0.577581f, 0.0f, 1.0f), FLinearColor(0.223228f, 0.0f, 0.001518f, 1.0f), alpha));
    }
    if (coefficient > 0.25f) {
        float alpha = (coefficient - 0.25f);
        return FLinearColor(FMath::Lerp(FLinearColor(0.0f, 1.0f, 0.270498f, 1.0f), FLinearColor(1.0f, 0.577581f, 0.0f, 1.0f), alpha));
    }
    else {
        float alpha = coefficient;
        return FLinearColor(FMath::Lerp(FLinearColor(0.708376f, 1.0f, 0.701102f, 1.0f), FLinearColor(0.0f, 1.0f, 0.270498f, 1.0f), FMath::Abs(alpha)));
    }
}

void UBulletComponent::SpawnHole(FHitResult HitResult)
{
    UDecalComponent* DecalComponent = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), DecalMaterial, DecalSize, HitResult.Location, UKismetMathLibrary::MakeRotFromX(HitResult.ImpactNormal), DecalLifeSpan);
    DecalComponent->FadeScreenSize = DecalFadeScreenSize;
}



