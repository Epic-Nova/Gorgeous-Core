// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousActionBar_CAW.h"

UGorgeousActionBar_CAW::UGorgeousActionBar_CAW(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// The Action Bar should be transparent to input by default
	// This ensures it doesn't steal focus or block the character control
	SetIsFocusable(false);
	
	// Default CommonUI properties to ensure it's treated as an overlay
	// Note: These might be overridden by the Blueprint defaults
}

TOptional<FUIInputConfig> UGorgeousActionBar_CAW::GetDesiredInputConfig() const
{
	// Return a config that allows Game input and doesn't capture the mouse
	// ECommonInputMode::Game means the UI doesn't block the game
	return FUIInputConfig(ECommonInputMode::Game, EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown);
}
