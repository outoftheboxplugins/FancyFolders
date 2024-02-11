// Copyright Out-of-the-Box Plugins 2018-2024. All Rights Reserved.

#include "FolderIconsStyle.h"

#include <Styling/SlateStyleRegistry.h>

#include "FolderIcons.h"

FFolderIconsStyle::FFolderIconsStyle() : FSlateStyleSet(TEXT("FolderIconsStyle"))
{
	const TArray<FString> IconFolders = FFolderIconsModule::GetIconFoldersOnDisk();
	for (const FString& Folder : IconFolders)
	{
		const FString Icon = FPaths::GetBaseFilename(Folder, true);

		const FName NormalIcon = *FString::Printf(TEXT("%s.Normal"), *Icon);
		Set(NormalIcon, new FSlateVectorImageBrush(Folder / TEXT("Normal.svg"), FVector2D(64, 64)));

		const FName ColumnOpenIcon = *FString::Printf(TEXT("%s.ColumnOpen"), *Icon);
		Set(ColumnOpenIcon, new FSlateVectorImageBrush(Folder / TEXT("ColumnOpen.svg"), FVector2D(16, 16)));

		const FName ColumnClosedIcon = *FString::Printf(TEXT("%s.ColumnClosed"), *Icon);
		Set(ColumnClosedIcon, new FSlateVectorImageBrush(Folder / TEXT("ColumnClosed.svg"), FVector2D(16, 16)));
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
