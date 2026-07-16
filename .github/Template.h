// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|                 Gorgeous XXX - XXX functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#pragma once

//<=============================--- Macros ---===============================>
//<-------------------------------------------------------------------------->

//<=============================--- Includes ---=============================>
//<--------------------------=== Game Includes ===--------------------------->
//<-------------------------=== Module Includes ===-------------------------->
//<-------------------------=== Engine Includes ===-------------------------->
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "....generated.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
//<-------------------------------------------------->

//<=================--- Delegates ---=================>
//<--------------------------------------------------->

//<=====================--- Class Macro Definitions ---======================>
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: -
| Functional Name: - 
| Parent Class: -
| Class Suffix: -
| Author: -
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| [Detailed description of the class's purpose, functionality, and context.]
|
| [Additional information about how this class interacts with other classes or systems.]
|
| @note [Any important notes or considerations.]
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS()
class _API ... : public ...
{
    GENERATED_BODY()

    //<===================--- Friend Classes ---================>
    //<--------------------------------------------------------->

    // Constructor [optional a descriptive thing if it does something special]
    ...();

    // Destructor [optional a descriptive thing if it does something special]
    virtual ~...();

    //<============================--- Overrides ---============================>
public:
s
    /**
     * [Description of the overridden function's purpose.]
     *
     * @param [Parameter name] [Parameter description.]
     * @return [Return value description.]
     */
    virtual void FunctionName() override;

protected:

    /**
     * [Description of the overridden function's purpose.]
     *
     * @param [Parameter name] [Parameter description.]
     * @return [Return value description.]
     */
    virtual void FunctionName() override;

private:

    /**
     * [Description of the overridden function's purpose.]
     *
     * @param [Parameter name] [Parameter description.]
     * @return [Return value description.]
     */
    virtual void FunctionName() override;

    //<----------------------=== Actor Overrides ===---------------------------->
    //<----------------------=== UObject Overrides ===-------------------------->
    //<--------------------=== Interface Overrides ===-------------------------->

    //<------------------------------------------------------------------------->


    //<============================--- Callbacks ---============================>
public:

    /**
     * [Description of the callback function's purpose.]
     *
     * @param [Parameter name] [Parameter description.]
     */
    void CallbackFunction();

protected:

    /**
     * [Description of the callback function's purpose.]
     *
     * @param [Parameter name] [Parameter description.]
     */
    void CallbackFunction();

private:

    /**
     * [Description of the callback function's purpose.]
     *
     * @param [Parameter name] [Parameter description.]
     */
    void CallbackFunction();
    //<------------------------------------------------------------------------->


    //<=======================--- Blueprint Functions ---=======================>
public:

    /**
     * [Description of the Blueprint function's purpose.]
     *
     * @param [Parameter name] [Parameter description.]
     * @return [Return value description.]
     */
    UFUNCTION(BlueprintCallable, Category = "Category")
    void BlueprintFunction();

protected:

    /**
     * [Description of the Blueprint function's purpose.]
     *
     * @param [Parameter name] [Parameter description.]
     * @return [Return value description.]
     */
    UFUNCTION(BlueprintCallable, Category = "Category")
    void BlueprintFunction();

private:

    /**
     * [Description of the Blueprint function's purpose.]
     *
     * @param [Parameter name] [Parameter description.]
     * @return [Return value description.]
     */
    UFUNCTION(BlueprintCallable, Category = "Category")
    void BlueprintFunction();
    //<------------------------------------------------------------------------->


    //<====================--- UAT/UBT Exposed Variables ---====================>
public:

    /**
     * [Description of the UAT/UBT exposed variable's purpose.]
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Category")
    int ExposedVariable;

protected:

    /**
     * [Description of the UAT/UBT exposed variable's purpose.]
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Category")
    int ExposedVariable;

private:

    /**
     * [Description of the UAT/UBT exposed variable's purpose.]
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Category")
    int ExposedVariable;
    //<------------------------------------------------------------------------->


    //<============================--- C++ Only ---=============================>
public:

    /**
     * [Description of the C++ only function's purpose.]
     *
     * @param [Parameter name] [Parameter description.]
     * @return [Return value description.]
     */
    void CppOnlyFunction();

protected:

    /**
     * [Description of the C++ only function's purpose.]
     *
     * @param [Parameter name] [Parameter description.]
     * @return [Return value description.]
     */
    void CppOnlyFunction();

private:

    /**
     * [Description of the C++ only function's purpose.]
     *
     * @param [Parameter name] [Parameter description.]
     * @return [Return value description.]
     */
    void CppOnlyFunction();
    //<------------------------------------------------------------------------->


    //<============================--- Variables ---============================>
public:

    // [Description of the variable's purpose.]
    int PublicVariable;

protected:

    // [Description of the variable's purpose.]
    int ProtectedVariable;

private:

     // [Description of the variable's purpose.]
    int PrivateVariable;
    //<------------------------------------------------------------------------->
};

//<=============================--- Inline Definitions ---=============================>
//<------------------------------------------------------------------------------------>