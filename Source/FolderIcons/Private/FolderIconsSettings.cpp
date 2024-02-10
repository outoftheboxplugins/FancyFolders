// My super cool copyright notice

#include "FolderIconsSettings.h"

#include "FolderIconsStyle.h"

const FSlateBrush* FFolderIconPreset::GetIcon(bool bIsColumnView) const
{
	const FString IconType = bIsColumnView ? TEXT("Column") : TEXT("Normal");
	const FName Brush = *FString::Printf(TEXT("%s.%s"), *Icon.ToString(), *IconType);
	return FFolderIconsStyle::Get().GetBrush(Brush);
}

FName UFolderIconsSettings::GetContainerName() const
{
	return TEXT("Editor");
}

FName UFolderIconsSettings::GetCategoryName() const
{
	return TEXT("Out-of-the-Box Plugins");
}
FName UFolderIconsSettings::GetSectionName() const
{
	return TEXT("Folder Icons");
}

#if WITH_EDITOR
FText UFolderIconsSettings::GetSectionText() const
{
	const FName DisplaySectionName = GetSectionName();
	return FText::FromName(DisplaySectionName);
}
#endif
