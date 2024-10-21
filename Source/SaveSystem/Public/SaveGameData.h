// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "GameFramework/SaveGame.h"
#include "GameplayTagContainer.h"
#include "SaveGameData.generated.h"

class UGameplayAbility;

USTRUCT()
struct FActorSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	FName Name;

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	TArray<uint8> ByteData;
};

USTRUCT()
struct FGameplayAbilitySaveData
{
	GENERATED_BODY()
	
	UPROPERTY()
	int32 Level;
	
	UPROPERTY()
	FGameplayTagContainer DynamicTags;

	UPROPERTY()
	TSubclassOf<UGameplayAbility> AbilityClass;
};

USTRUCT(BlueprintType)
struct FSaveGameMetadata
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UTexture2D> Screenshot;
};

/**
 * 
 */
UCLASS()
class SAVESYSTEM_API USaveGameData : public USaveGame
{
	GENERATED_BODY()

public:
	/**
	 * If the screenshot is a separate file, this variable will store the path to that screenshot.
	 * Otherwise, this variable stores the screenshot binary data.
	 */
	UPROPERTY(AssetRegistrySearchable)
	FString ScreenshotData;
	
	UPROPERTY()
	TArray<FActorSaveData> SavedActors;

	UPROPERTY()
	TArray<FGameplayAbilitySaveData> SavedPlayerAbilities;
	
	UPROPERTY()
	TArray<uint8> ScreenshotBytes;

	UFUNCTION(BlueprintCallable, Category = "Save System", meta = (WorldContext = "WorldContextObject"))
	UTexture2D* GetScreenshotTexture(UObject* WorldContextObject) const;
};
