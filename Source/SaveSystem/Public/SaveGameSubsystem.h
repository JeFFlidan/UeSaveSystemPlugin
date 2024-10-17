// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveGameSubsystem.generated.h"

class USaveGameData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReadWriteSaveGame, USaveGameData*, SaveGameObj);

/**
 * 
 */
UCLASS()
class SAVESYSTEM_API USaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnReadWriteSaveGame OnSaveGameLoaded;

	UPROPERTY(BlueprintAssignable)
	FOnReadWriteSaveGame OnSaveGameWritten;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable)
	void SetSlotName(FString NewSlotName);

	UFUNCTION(BlueprintCallable)
	void WriteSaveGame(FString InSlotName = "");

	UFUNCTION(BlueprintCallable)
	void LoadSaveGame(FString InSlotName = "");

protected:
	UPROPERTY()
	TObjectPtr<USaveGameData> CurrentSaveGame;

	FString CurrentSlotName;
};
