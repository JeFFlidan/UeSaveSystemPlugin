// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "SaveGameSubsystem.h"
#include "SaveSystemSettings.h"
#include "SaveGameData.h"
#include "SavableActorInterface.h"
#include "SaveSystemLogChannels.h"

#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SaveGameSubsystem)

void USaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const USaveSystemSettings* Settings = GetDefault<USaveSystemSettings>();
	CurrentSlotName = Settings->DefaultSaveSlotName;
}

void USaveGameSubsystem::SetSlotName(FString NewSlotName)
{
	if (NewSlotName.IsEmpty())
	{
		return;
	}

	CurrentSlotName = NewSlotName;
}

void USaveGameSubsystem::WriteSaveGame(FString InSlotName)
{
	SetSlotName(InSlotName);
	
	CurrentSaveGame->SavedActors.Empty();

	for (AActor* Actor : TActorRange<AActor>(GetWorld()))
	{
		if (!IsValid(Actor) || !Actor->Implements<USavableActorInterface>())
		{
			continue;
		}

		FActorSaveData ActorData;
		ActorData.Name = Actor->GetFName();
		ActorData.Transform = Actor->GetActorTransform();

		FMemoryWriter MemWriter(ActorData.ByteData);
		FObjectAndNameAsStringProxyArchive Archive(MemWriter, true);
		Archive.ArIsSaveGame = true;
		Actor->Serialize(Archive);

		CurrentSaveGame->SavedActors.Add(ActorData);
	}

	UGameplayStatics::SaveGameToSlot(CurrentSaveGame, CurrentSlotName, 0);
	UE_LOG(LogSaveSystem, Log, TEXT("Wrote SaveGameData to slot %s"), *CurrentSlotName)
	
	OnSaveGameWritten.Broadcast(CurrentSaveGame);
}

void USaveGameSubsystem::LoadSaveGame(FString InSlotName)
{
	SetSlotName(InSlotName);

	if (UGameplayStatics::DoesSaveGameExist(CurrentSlotName, 0))
	{
		CurrentSaveGame = Cast<USaveGameData>(UGameplayStatics::LoadGameFromSlot(CurrentSlotName, 0));
		if (!CurrentSaveGame)
		{
			UE_LOG(LogSaveSystem, Warning, TEXT("Failed to load SaveGameData slot %s"), *CurrentSlotName);
			return;
		}

		UE_LOG(LogSaveSystem, Log, TEXT("Loaded SaveGameData from slot %s"), *CurrentSlotName);

		for (AActor* Actor : TActorRange<AActor>(GetWorld()))
		{
			if (!Actor->Implements<USavableActorInterface>())
			{
				continue;
			}

			for (FActorSaveData ActorData : CurrentSaveGame->SavedActors)
			{
				if (ActorData.Name == Actor->GetFName())
				{
					Actor->SetActorTransform(ActorData.Transform);

					FMemoryReader MemReader(ActorData.ByteData);
					FObjectAndNameAsStringProxyArchive Archive(MemReader, true);
					Archive.ArIsSaveGame = true;
					Actor->Serialize(Archive);
					ISavableActorInterface::Execute_OnActorLoaded(Actor);

					break;
				}
			}
		}

		OnSaveGameLoaded.Broadcast(CurrentSaveGame);
	}
	else
	{
		CurrentSaveGame = CastChecked<USaveGameData>(UGameplayStatics::CreateSaveGameObject(USaveGameData::StaticClass()));
		UE_LOG(LogSaveSystem, Log, TEXT("Created new SaveGameData object"));
	}
}
