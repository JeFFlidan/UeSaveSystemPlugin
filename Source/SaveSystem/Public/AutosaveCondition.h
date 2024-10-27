// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "AutosaveCondition.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class SAVESYSTEM_API UAutosaveCondition : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	bool IsAutosavePossible() const;

	virtual bool IsAutosavePossible_Implementation() const { return true; }
};
