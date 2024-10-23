// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveGameSubsystem.generated.h"

class USaveGameData;
class UScreenshotTaker;
class USaveSystemSettings;
class USaveGameMetadata;
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

	UFUNCTION(BlueprintCallable, Category = "Save System")
	virtual const TArray<USaveGameMetadata*>& LoadAllSaveGameMetadata();

	UFUNCTION(BlueprintPure, Category = "Save System")
	virtual const TArray<USaveGameMetadata*>& GetCachedGameSaveMetadata() const { return LoadedMetadata; }

protected:
	UPROPERTY()
	TObjectPtr<USaveGameData> CurrentSaveGame;

	UPROPERTY()
	TObjectPtr<USaveGameMetadata> MetadataCDO;
	
	UPROPERTY()
	TObjectPtr<UScreenshotTaker> ScreenshotTaker;

	UPROPERTY()
	TObjectPtr<const USaveSystemSettings> Settings;

	UPROPERTY()
	TArray<TObjectPtr<USaveGameMetadata>> LoadedMetadata;

	FString CurrentSlotName;
	FString CurrentMetadataFilename;
	
	bool bShouldSaveInDelegate;

	virtual void SaveAbilitySystemState();
	virtual UAbilitySystemComponent* FindPlayerAbilitySystemComponent() const;

	UFUNCTION()
	virtual void HandleScreenshotTaken(const TArray<uint8>& ScreenshotBytes);

	void SaveGameToSlot() ;
	void SaveMetadata();
	USaveGameMetadata* ReadMetadata(const FString& MetadataPath) const;
	UTexture2D* LoadScreenshot(const FString& MetadataPath) const;
	
	bool CanRequestScreenshot() const;
	FString GetSaveDirectory() const;
	FString GetScreenshotFilename() const;
	FString GetScreenshotFormat() const;
};
