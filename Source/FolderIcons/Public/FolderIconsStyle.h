// Copyright Out-of-the-Box Plugins 2018-2024. All Rights Reserved.

#pragma once

#include <Styling/SlateStyle.h>

class FFolderIconsStyle final : public FSlateStyleSet
{
public:
	FFolderIconsStyle();
	virtual ~FFolderIconsStyle() override;

	static FFolderIconsStyle& Get();
};