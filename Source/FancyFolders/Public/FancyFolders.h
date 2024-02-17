// Copyright Out-of-the-Box Plugins 2018-2024. All Rights Reserved.

#pragma once

#include <Modules/ModuleInterface.h>

class UContentBrowserFolderContext;

/**
 * Module responsible for allowing developers to set icons to folders
 */
class FFancyFoldersModule : public IModuleInterface
{
public:
	/**
	 * Returns all the available icons on disk from the Resources folder
	 */
	static TArray<FString> GetIconFoldersOnDisk();

private:
	// Begin IModuleInterface interface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	// End IModuleInterface interface
	/**
	 * Callback executed when the context menu is built to inject the IconSelection section
	 */
	void ExtendFolderContextMenu(UToolMenu* InMenu);
	/**
	 * Callback executed to build the IconSelection entries in the context menu
	 */
	void BuildContextMenu(FMenuBuilder& MenuBuilder, UContentBrowserFolderContext* Context);
};
