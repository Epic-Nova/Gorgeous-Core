#pragma once

#include "UObject/NameTypes.h"

class ISettingsModule;

namespace GorgeousCoreSettingsPages
{
	inline const FName EditorDiagnosticsViewId(TEXT("CoreDebuggingDiagnostics"));

	void Register(ISettingsModule& Settings);
	void Unregister(ISettingsModule* Settings);
}
