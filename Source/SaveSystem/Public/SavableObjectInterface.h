// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SavableObjectInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USavableObjectInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SAVESYSTEM_API ISavableObjectInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent)
	void OnObjectLoaded();
};
