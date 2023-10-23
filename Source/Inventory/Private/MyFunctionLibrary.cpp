// Fill out your copyright notice in the Description page of Project Settings.


#include "MyFunctionLibrary.h"
#include "Misc/DateTime.h"


bool UMyFunctionLibrary::CustomForLoop(FDelegateTypeTest delegatevariable2, int32 NumberOfIterations)
{
    for (int32 i = 0; i < NumberOfIterations; i++)
    {
        // Вызываем колбэк-функцию на каждой итерации
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("%d"), i));
        //delegatevariable2.ExecuteIfBound(i);
    }
    
    return true;
}
