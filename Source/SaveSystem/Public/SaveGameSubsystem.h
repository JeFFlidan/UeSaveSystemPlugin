// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveGameSubsystem.generated.h"

class USaveGameData;
class UScreenshotTaker;
class UAbilitySystemComponent;

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

	UFUNCTION(BlueprintCallable, Category = "Save System")
	void SetSlotName(FString NewSlotName);

	UFUNCTION(BlueprintCallable, Category = "Save System")
	virtual void WriteSaveGame(FString InSlotName = "");

	UFUNCTION(BlueprintCallable, Category = "Save System")
	virtual void LoadSaveGame(FString InSlotName = "");

	UFUNCTION(BlueprintCallable, Category = "Save System")
	virtual void LoadPlayerAbilitySystemState();

protected:
	UPROPERTY()
	TObjectPtr<USaveGameData> CurrentSaveGame;

	UPROPERTY()
	TObjectPtr<UScreenshotTaker> ScreenshotTaker;

	FString CurrentSlotName;

	bool bShouldTakeScreenshot;
	bool bShouldSaveInDelegate;

	virtual void SaveAbilitySystemState();
	virtual UAbilitySystemComponent* FindPlayerAbilitySystemComponent() const;

	UFUNCTION()
	virtual void HandleScreenshotTaken(const TArray<uint8>& ScreenshotData);

	void SaveGameToSlot() const;
	bool CanRequestScreenshot() const { return bShouldTakeScreenshot && ScreenshotTaker; }
};
