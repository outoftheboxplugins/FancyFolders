// My super cool copyright notice

#include "FancyFoldersSettings.h"

#include "FancyFoldersStyle.h"

const FSlateBrush* FFolderIconPreset::GetIcon(bool bIsColumnView, bool bIsOpen) const
{
	const FString IconType = [=]()
	{
		if (bIsColumnView)
		{
			if (bIsOpen)
			{
				return TEXT("ColumnOpen");
			}

			return TEXT("ColumnClosed");
		}
		return TEXT("Normal");
	}();

	const FName Brush = *FString::Printf(TEXT("%s.%s"), *Icon.ToString(), *IconType);
	return FFancyFoldersStyle::Get().GetBrush(Brush);
}

FName UFancyFoldersSettings::GetContainerName() const
{
	return TEXT("Editor");
}

FName UFancyFoldersSettings::GetCategoryName() const
{
	return TEXT("Out-of-the-Box Plugins");
}
FName UFancyFoldersSettings::GetSectionName() const
{
	return TEXT("Folder Icons");
}

#if WITH_EDITOR
FText UFancyFoldersSettings::GetSectionText() const
{
	const FName DisplaySectionName = GetSectionName();
	return FText::FromName(DisplaySectionName);
}
#endif
