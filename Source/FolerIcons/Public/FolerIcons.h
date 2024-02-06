// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class UContentBrowserFolderContext;

class FFolerIconsModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void OnApplicationTick(float DeltaTime);

	void BuildContextMenu(FMenuBuilder& MenuBuilder, UContentBrowserFolderContext* Context);

	void AddIconsToWidgets();
	const FSlateBrush* GetIconForFolder(FString VirtualPath) const;

	TMap<FString, TSharedRef<SWidget>> CurrentAssetWidgets;
	TMap<FString, FName> PresetIcons;
	TMap<FString, FName> Icons;
};
