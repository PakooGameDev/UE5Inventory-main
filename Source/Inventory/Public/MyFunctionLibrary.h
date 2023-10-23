// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MyFunctionLibrary.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_DELEGATE_OneParam(FDelegateTypeTest, int32, Index);

UCLASS()
class INVENTORY_API UMyFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	UPROPERTY()
		FDelegateTypeTest DelegateVariable;
	UFUNCTION(BlueprintCallable, Category = "Loops")
		static bool CustomForLoop(FDelegateTypeTest delegatevariable2, int32 NumberOfIterations);


};

