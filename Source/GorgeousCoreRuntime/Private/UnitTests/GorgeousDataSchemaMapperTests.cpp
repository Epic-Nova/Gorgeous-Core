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

#if WITH_DEV_AUTOMATION_TESTS

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "DataSchemaMapping/GorgeousDataSchemaMapper.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "Misc/AutomationTest.h"
//<-------------------------------------------------------------------------->

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousDataSchemaTrimWhitespaceTransformTest, "GorgeousCore.DataSchemaMapping.Transform.TrimWhitespace", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousDataSchemaTrimWhitespaceTransformTest::RunTest(const FString& Parameters)
{
	FGorgeousDataSchemaTransformDefinition_S TransformDefinition;
	TransformDefinition.TransformKind = EGorgeousDataSchemaTransformKind_E::Transform_TrimWhitespace;

	FString OutputValue;
	FString ErrorMessage;
	const bool bSucceeded = FGorgeousDataSchemaMapper::ApplyTransformDefinition(TransformDefinition, TEXT("  Item Label  "), OutputValue, ErrorMessage);
	TestTrue(TEXT("Trim whitespace transform succeeds"), bSucceeded);
	TestTrue(TEXT("Trim whitespace transform does not return an error"), ErrorMessage.IsEmpty());
	TestEqual(TEXT("Trim whitespace transform strips both sides"), OutputValue, FString(TEXT("Item Label")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousDataSchemaReplaceTextTransformTest, "GorgeousCore.DataSchemaMapping.Transform.ReplaceText", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousDataSchemaReplaceTextTransformTest::RunTest(const FString& Parameters)
{
	FGorgeousDataSchemaTransformDefinition_S TransformDefinition;
	TransformDefinition.TransformKind = EGorgeousDataSchemaTransformKind_E::Transform_ReplaceText;
	TransformDefinition.ParameterA = TEXT("Old");
	TransformDefinition.ParameterB = TEXT("New");

	FString OutputValue;
	FString ErrorMessage;
	const bool bSucceeded = FGorgeousDataSchemaMapper::ApplyTransformDefinition(TransformDefinition, TEXT("OldName_OldValue"), OutputValue, ErrorMessage);
	TestTrue(TEXT("Replace text transform succeeds"), bSucceeded);
	TestTrue(TEXT("Replace text transform does not return an error"), ErrorMessage.IsEmpty());
	TestEqual(TEXT("Replace text transform replaces all exact matches"), OutputValue, FString(TEXT("NewName_NewValue")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousDataSchemaMultiplyNumericTransformTest, "GorgeousCore.DataSchemaMapping.Transform.MultiplyNumeric", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousDataSchemaMultiplyNumericTransformTest::RunTest(const FString& Parameters)
{
	FGorgeousDataSchemaTransformDefinition_S TransformDefinition;
	TransformDefinition.TransformKind = EGorgeousDataSchemaTransformKind_E::Transform_MultiplyNumeric;
	TransformDefinition.ParameterA = TEXT("2.5");

	FString OutputValue;
	FString ErrorMessage;
	const bool bSucceeded = FGorgeousDataSchemaMapper::ApplyTransformDefinition(TransformDefinition, TEXT("4"), OutputValue, ErrorMessage);
	TestTrue(TEXT("Multiply numeric transform succeeds"), bSucceeded);
	TestTrue(TEXT("Multiply numeric transform does not return an error"), ErrorMessage.IsEmpty());
	TestEqual(TEXT("Multiply numeric result is correct"), FCString::Atod(*OutputValue), 10.0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousDataSchemaAddNumericOffsetTransformTest, "GorgeousCore.DataSchemaMapping.Transform.AddNumericOffset", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousDataSchemaAddNumericOffsetTransformTest::RunTest(const FString& Parameters)
{
	FGorgeousDataSchemaTransformDefinition_S TransformDefinition;
	TransformDefinition.TransformKind = EGorgeousDataSchemaTransformKind_E::Transform_AddNumericOffset;
	TransformDefinition.ParameterA = TEXT("1.25");

	FString OutputValue;
	FString ErrorMessage;
	const bool bSucceeded = FGorgeousDataSchemaMapper::ApplyTransformDefinition(TransformDefinition, TEXT("3.75"), OutputValue, ErrorMessage);
	TestTrue(TEXT("Add numeric offset transform succeeds"), bSucceeded);
	TestTrue(TEXT("Add numeric offset transform does not return an error"), ErrorMessage.IsEmpty());
	TestEqual(TEXT("Add numeric offset result is correct"), FCString::Atod(*OutputValue), 5.0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousDataSchemaSetLiteralTransformTest, "GorgeousCore.DataSchemaMapping.Transform.SetLiteral", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousDataSchemaSetLiteralTransformTest::RunTest(const FString& Parameters)
{
	FGorgeousDataSchemaTransformDefinition_S TransformDefinition;
	TransformDefinition.TransformKind = EGorgeousDataSchemaTransformKind_E::Transform_SetLiteral;
	TransformDefinition.ParameterA = TEXT("42");

	FString OutputValue;
	FString ErrorMessage;
	const bool bSucceeded = FGorgeousDataSchemaMapper::ApplyTransformDefinition(TransformDefinition, TEXT("12"), OutputValue, ErrorMessage);
	TestTrue(TEXT("Set literal transform succeeds"), bSucceeded);
	TestTrue(TEXT("Set literal transform does not return an error"), ErrorMessage.IsEmpty());
	TestEqual(TEXT("Set literal transform overwrites source value"), OutputValue, FString(TEXT("42")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousDataSchemaFallbackLiteralTransformTest, "GorgeousCore.DataSchemaMapping.Transform.FallbackLiteralIfEmpty", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousDataSchemaFallbackLiteralTransformTest::RunTest(const FString& Parameters)
{
	FGorgeousDataSchemaTransformDefinition_S TransformDefinition;
	TransformDefinition.TransformKind = EGorgeousDataSchemaTransformKind_E::Transform_UseFallbackLiteralIfEmpty;
	TransformDefinition.ParameterA = TEXT("FallbackValue");

	FString OutputValue;
	FString ErrorMessage;
	bool bSucceeded = FGorgeousDataSchemaMapper::ApplyTransformDefinition(TransformDefinition, TEXT(""), OutputValue, ErrorMessage);
	TestTrue(TEXT("Fallback transform succeeds for empty source"), bSucceeded);
	TestEqual(TEXT("Fallback transform uses fallback literal for empty source"), OutputValue, FString(TEXT("FallbackValue")));

	bSucceeded = FGorgeousDataSchemaMapper::ApplyTransformDefinition(TransformDefinition, TEXT("ActualValue"), OutputValue, ErrorMessage);
	TestTrue(TEXT("Fallback transform succeeds for non-empty source"), bSucceeded);
	TestEqual(TEXT("Fallback transform keeps non-empty source"), OutputValue, FString(TEXT("ActualValue")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousDataSchemaInvertBooleanTransformTest, "GorgeousCore.DataSchemaMapping.Transform.InvertBoolean", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousDataSchemaInvertBooleanTransformTest::RunTest(const FString& Parameters)
{
	FGorgeousDataSchemaTransformDefinition_S TransformDefinition;
	TransformDefinition.TransformKind = EGorgeousDataSchemaTransformKind_E::Transform_InvertBoolean;

	FString OutputValue;
	FString ErrorMessage;
	bool bSucceeded = FGorgeousDataSchemaMapper::ApplyTransformDefinition(TransformDefinition, TEXT("true"), OutputValue, ErrorMessage);
	TestTrue(TEXT("Invert boolean succeeds for true"), bSucceeded);
	TestEqual(TEXT("Invert boolean maps true->False"), OutputValue, FString(TEXT("False")));

	bSucceeded = FGorgeousDataSchemaMapper::ApplyTransformDefinition(TransformDefinition, TEXT("0"), OutputValue, ErrorMessage);
	TestTrue(TEXT("Invert boolean succeeds for 0"), bSucceeded);
	TestEqual(TEXT("Invert boolean maps 0->True"), OutputValue, FString(TEXT("True")));

	bSucceeded = FGorgeousDataSchemaMapper::ApplyTransformDefinition(TransformDefinition, TEXT("maybe"), OutputValue, ErrorMessage);
	TestFalse(TEXT("Invert boolean fails for invalid input"), bSucceeded);
	TestTrue(TEXT("Invert boolean returns error for invalid input"), ErrorMessage.Contains(TEXT("boolean")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousDataSchemaNumericTransformRejectsInvalidInputTest, "GorgeousCore.DataSchemaMapping.Transform.NumericValidation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousDataSchemaNumericTransformRejectsInvalidInputTest::RunTest(const FString& Parameters)
{
	FGorgeousDataSchemaTransformDefinition_S TransformDefinition;
	TransformDefinition.TransformKind = EGorgeousDataSchemaTransformKind_E::Transform_MultiplyNumeric;
	TransformDefinition.ParameterA = TEXT("2");

	FString OutputValue;
	FString ErrorMessage;
	const bool bSucceeded = FGorgeousDataSchemaMapper::ApplyTransformDefinition(TransformDefinition, TEXT("NotANumber"), OutputValue, ErrorMessage);
	TestFalse(TEXT("Numeric transform fails for invalid source text"), bSucceeded);
	TestTrue(TEXT("Numeric transform returns a useful error"), ErrorMessage.Contains(TEXT("not numeric")));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
