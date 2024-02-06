// My super cool copyright notice

#include "FolderIconsSettings.h"

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
