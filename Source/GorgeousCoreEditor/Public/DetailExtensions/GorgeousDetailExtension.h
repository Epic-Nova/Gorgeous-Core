// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "UObject/NoExportTypes.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "PropertyHandle.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousDetailExtension.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Base class for modular property customizations in the Unreal Editor.F
 * Can be triggered via metadata: meta = (GorgeousDetailExtension = "ExtensionName")
 */
UCLASS(Abstract)
class GORGEOUSCOREEDITOR_API UGorgeousDetailExtension : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Called to customize the header of a property.
	 * @param PropertyHandle Handle to the property being customized.
	 * @param HeaderRow The Slate row for the property header.
	 */
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow)
	{
		HeaderRow.NameContent()[PropertyHandle->CreatePropertyNameWidget()].ValueContent()[PropertyHandle->CreatePropertyValueWidget()];
	}

	/**
	 * Called to customize the children of a property.
	 * @param PropertyHandle Handle to the property being customized.
	 * @param ChildBuilder Builder for child properties.
	 */
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder)
	{
		uint32 Count = 0;
		PropertyHandle->GetNumChildren(Count);
		for (uint32 Index = 0; Index < Count; ++Index)
		{
			if (TSharedPtr<IPropertyHandle> Child = PropertyHandle->GetChildHandle(Index); Child.IsValid() && Child->IsValidHandle()) ChildBuilder.AddProperty(Child.ToSharedRef());
		}
	}

	/** Returns the unique identifier for this extension. */
	virtual FName GetExtensionName() const { return NAME_None; }
};