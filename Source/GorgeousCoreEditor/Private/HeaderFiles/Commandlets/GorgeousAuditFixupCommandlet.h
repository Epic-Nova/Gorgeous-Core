// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "Commandlets/Commandlet.h"
#include "GorgeousAuditFixupCommandlet.generated.h"

/** Validates bundled guard data and Blueprint compilation; -Apply persists explicit repairs. */
UCLASS()
class UGorgeousAuditFixupCommandlet : public UCommandlet
{
	GENERATED_BODY()
public:
	UGorgeousAuditFixupCommandlet();
	virtual int32 Main(const FString& Params) override;
};
