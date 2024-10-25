// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "GameFramework/SaveGame.h"
#include "GameplayTagContainer.h"
#include "SaveGameData.generated.h"

class UGameplayAbility;
class UGameplayEffect;

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

USTRUCT()
struct FGameplayEffectSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	float Level;

	UPROPERTY()
	TSubclassOf<UGameplayEffect> EffectClass;
};

USTRUCT()
struct FAttributeSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	float BaseValue;
};

/**
 * 
 */
UCLASS()
class SAVESYSTEM_API USaveGameData : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FActorSaveData> SavedActors;

	UPROPERTY()
	TArray<FGameplayAbilitySaveData> SavedPlayerAbilities;

	UPROPERTY()
	TArray<FGameplayEffectSaveData> SavedGameplayEffects;

	// Key has the structure HealthSet.Health
	UPROPERTY()
	TMap<FString, FAttributeSaveData> SavedAttributes;
};
