// Copyright Out-of-the-Box Plugins 2018-2024. All Rights Reserved.

#include "FolderIconsStyle.h"

#include <Styling/SlateStyleRegistry.h>

#include "FolderIcons.h"

FFolderIconsStyle::FFolderIconsStyle() : FSlateStyleSet(TEXT("FolderIconsStyle"))
{
	const TArray<FString> FolderIcons = FFolderIconsModule::GetFolderIconsOnDisk();
	for (const FString& File : FolderIcons)
	{
		const FName Property = *FPaths::GetBaseFilename(File, true);
		Set(Property, new FSlateVectorImageBrush(File, FVector2D(64, 64)));
	}

	FSlateStyleRegistry::RegisterSlateStyle(*this);
}

FFolderIconsStyle& FFolderIconsStyle::Get()
{
	static FFolderIconsStyle Inst;
	return Inst;
}

FFolderIconsStyle::~FFolderIconsStyle()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*this);
}
