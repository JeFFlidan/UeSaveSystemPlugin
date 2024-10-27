// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "SaveGameMetadata.generated.h"

class UTexture2D;

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class SAVESYSTEM_API USaveGameMetadata : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Metadata", Transient)
	TObjectPtr<UTexture2D> Screenshot;

	UPROPERTY(BlueprintReadOnly, Category = "Metadata")
	FText SlotName;

	virtual void InitMetadata();

	UFUNCTION(BlueprintImplementableEvent, Category = "Save Game | Metadata")
	void BP_InitMetadata();
};
