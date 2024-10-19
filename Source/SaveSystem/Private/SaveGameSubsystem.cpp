// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "SaveGameSubsystem.h"
#include "SaveSystemSettings.h"
#include "SaveGameData.h"
#include "SavableObjectInterface.h"
#include "SaveSystemLogChannels.h"

#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"
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
		if (!IsValid(Actor) || !Actor->Implements<USavableObjectInterface>())
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

	SaveAbilitySystemState();

	UGameplayStatics::SaveGameToSlot(CurrentSaveGame, CurrentSlotName, 0);
	UE_LOG(LogSaveSystem, Display, TEXT("Wrote SaveGameData to slot %s"), *CurrentSlotName)
	
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

		UE_LOG(LogSaveSystem, Display, TEXT("Loaded SaveGameData from slot %s"), *CurrentSlotName);

		for (AActor* Actor : TActorRange<AActor>(GetWorld()))
		{
			if (!Actor->Implements<USavableObjectInterface>())
			{
				continue;
			}

			bool bIsActorValid = false;
			
			for (FActorSaveData ActorData : CurrentSaveGame->SavedActors)
			{
				if (ActorData.Name == Actor->GetFName())
				{
					Actor->SetActorTransform(ActorData.Transform);

					FMemoryReader MemReader(ActorData.ByteData);
					FObjectAndNameAsStringProxyArchive Archive(MemReader, true);
					Archive.ArIsSaveGame = true;
					Actor->Serialize(Archive);
					ISavableObjectInterface::Execute_OnActorLoaded(Actor);

					bIsActorValid = true;

					break;
				}
			}

			if (!bIsActorValid)
			{
				Actor->Destroy();
			}
		}

		OnSaveGameLoaded.Broadcast(CurrentSaveGame);
	}
	else
	{
		CurrentSaveGame = CastChecked<USaveGameData>(UGameplayStatics::CreateSaveGameObject(USaveGameData::StaticClass()));
		UE_LOG(LogSaveSystem, Display, TEXT("Created new SaveGameData object"));
	}
}

void USaveGameSubsystem::LoadPlayerAbilitySystemState()
{
	UAbilitySystemComponent* ASC = FindPlayerAbilitySystemComponent();

	if (!ASC)
	{
		return;
	}
	UE_LOG(LogSaveSystem, Display, TEXT("Ability data size: %d"), CurrentSaveGame->SavedPlayerAbilities.Num())
	for (const FGameplayAbilitySaveData& AbilityData : CurrentSaveGame->SavedPlayerAbilities)
	{
		UGameplayAbility* AbilityCDO = AbilityData.AbilityClass->GetDefaultObject<UGameplayAbility>();

		FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityData.Level);
		AbilitySpec.DynamicAbilityTags = AbilityData.DynamicTags;

		ASC->GiveAbility(AbilitySpec);
	}
}

void USaveGameSubsystem::SaveAbilitySystemState()
{
	UAbilitySystemComponent* ASC = FindPlayerAbilitySystemComponent();

	if (!ASC)
	{
		return;
	}

	const TArray<FGameplayAbilitySpec>& AbilitySpecs = ASC->GetActivatableAbilities();
	for (const FGameplayAbilitySpec& Spec : AbilitySpecs)
	{
		UGameplayAbility* Ability = Spec.Ability;

		if (!Ability->Implements<USavableObjectInterface>())
		{
			continue;
		}

		FGameplayAbilitySaveData AbilityData;
		AbilityData.AbilityClass = Ability->GetClass();
		AbilityData.Level = Ability->GetAbilityLevel();
		AbilityData.DynamicTags = Spec.DynamicAbilityTags;
		
		CurrentSaveGame->SavedPlayerAbilities.Add(AbilityData);
	}
}

UAbilitySystemComponent* USaveGameSubsystem::FindPlayerAbilitySystemComponent() const
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(GetWorld(), 0);
	
	if (UAbilitySystemComponent* ASC = PlayerState->FindComponentByClass<UAbilitySystemComponent>())
	{
		return ASC;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (UAbilitySystemComponent* ASC = PlayerPawn->FindComponentByClass<UAbilitySystemComponent>())
	{
		return ASC;
	}

	return nullptr;
}
