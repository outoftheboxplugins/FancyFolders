// Copyright Out-of-the-Box Plugins 2018-2024. All Rights Reserved.

#pragma once

#include <Modules/ModuleInterface.h>

class UContentBrowserFolderContext;

class FFolderIconsModule : public IModuleInterface
{
public:
	static FString GetResourcesFolder();
	static TArray<FString> GetFolderIconsOnDisk();

private:
	// Begin IModuleInterface interface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	// End IModuleInterface interface


	// TODO: Below might require a refactor
	void OnApplicationTick(float DeltaTime);

	void BuildContextMenu(FMenuBuilder& MenuBuilder, UContentBrowserFolderContext* Context);

	void AddIconsToWidgets();
	const FSlateBrush* GetIconForFolder(FString VirtualPath) const;

	TMap<FString, TSharedRef<SWidget>> CurrentAssetWidgets;
	TMap<FString, FName> PresetIcons;
	TMap<FString, FName> Icons;
};
