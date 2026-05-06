// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PropertyHandle.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "GorgeousDetailExtension.generated.h"

/**
 * Base class for modular property customizations in the Unreal Editor.
 * Can be triggered via metadata: meta = (GorgeousDetailExtension = "ExtensionName")
 */
UCLASS(Abstract, Blueprintable)
class GORGEOUSCOREEDITOR_API UGorgeousDetailExtension : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Called to customize the header of a property.
	 * @param PropertyHandle Handle to the property being customized.
	 * @param HeaderRow The Slate row for the property header.
	 */
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow) {}

	/**
	 * Called to customize the children of a property.
	 * @param PropertyHandle Handle to the property being customized.
	 * @param ChildBuilder Builder for child properties.
	 */
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder) {}

	/** Returns the unique identifier for this extension. */
	virtual FName GetExtensionName() const { return NAME_None; }
};
