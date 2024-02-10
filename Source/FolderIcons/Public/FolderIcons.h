// Copyright Out-of-the-Box Plugins 2018-2024. All Rights Reserved.

#pragma once

#include <Modules/ModuleInterface.h>

class UContentBrowserFolderContext;

class FFolderIconsModule : public IModuleInterface
{
public:
	static TArray<FString> GetIconFoldersOnDisk();

private:
	// Begin IModuleInterface interface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	// End IModuleInterface interface

	/**
	 * Callback executed to dynamically build a folder's right click menu
	 */
	void BuildContextMenu(FMenuBuilder& MenuBuilder, UContentBrowserFolderContext* Context);
};
