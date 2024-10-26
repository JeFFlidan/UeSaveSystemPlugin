// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "AutosaveCondition.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class SAVESYSTEM_API UAutosaveCondition : public UObject
{
	GENERATED_BODY()

public:
	virtual bool IsAutosavePossible() const { return true; }
};
