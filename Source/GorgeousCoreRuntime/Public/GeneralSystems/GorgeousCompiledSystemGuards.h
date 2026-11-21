#pragma once

#include "UObject/NameTypes.h"

namespace GorgeousCompiledSystemGuards
{
	/** Unknown or inactive guards are unavailable in this target. */
	GORGEOUSCORERUNTIME_API bool IsAvailable(FName Guard);
}
