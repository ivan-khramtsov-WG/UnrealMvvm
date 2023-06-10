// Copyright Andrei Sudarikov. All Rights Reserved.

#include "K2Node_ViewModelPropertyBase.h"
#include "Mvvm/Impl/ViewModelRegistry.h"
#include "Mvvm/ViewModelProperty.h"
#include "ViewModelPropertyNodeHelper.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EditorCategoryUtils.h"

bool UK2Node_ViewModelPropertyBase::Modify(bool bAlwaysMarkDirty)
{
    CachedTexts.MarkDirty();

    return Super::Modify(bAlwaysMarkDirty);
}

FText UK2Node_ViewModelPropertyBase::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    if (!CachedTexts.IsTitleCached(TitleType, this))
    {
        CachedTexts.SetCachedTitle(TitleType, GetNodeTitleForCache(TitleType), this);
    }

    return CachedTexts.GetCachedTitle(TitleType);
}

FText UK2Node_ViewModelPropertyBase::GetTooltipText() const
{
    if (!CachedTexts.IsTooltipCached(this))
    {
        CachedTexts.SetCachedTooltip(GetTooltipTextForCache(), this);
    }

    return CachedTexts.GetCachedTooltip();
}

FSlateIcon UK2Node_ViewModelPropertyBase::GetIconAndTint(FLinearColor& OutColor) const
{
    OutColor = GetNodeTitleColor();

    static FSlateIcon Icon(FAppStyle::GetAppStyleSetName(), "Kismet.AllClasses.FunctionIcon");
    return Icon;
}

void UK2Node_ViewModelPropertyBase::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
    Super::ValidateNodeDuringCompilation(MessageLog);

    using namespace UnrealMvvm_Impl;

    const FViewModelPropertyReflection* PropertyInfo = FViewModelRegistry::FindProperty(ViewModelOwnerClass, ViewModelPropertyName);
    if (!PropertyInfo)
    {
        FText Message = FText::Format(
            NSLOCTEXT("UnrealMvvm", "Error.InvalidProperty", "@@ - Property {ViewModel}.{PropertyName} does not exist"),
            FText::FromString(GetNameSafe(ViewModelOwnerClass)),
            FText::FromName(ViewModelPropertyName)
        );

        MessageLog.Error(*Message.ToString(), this);
        return;
    }

    if (!FViewModelPropertyNodeHelper::IsPropertyAvailableInBlueprint(*PropertyInfo))
    {
        FText Message = FText::Format(
            NSLOCTEXT("UnrealMvvm", "Error.PropertyNotAvailableInBlueprint", "@@ - Property {ViewModel}.{PropertyName} is not available in Blueprint"),
            FText::FromString(GetNameSafe(ViewModelOwnerClass)),
            FText::FromName(ViewModelPropertyName)
        );

        MessageLog.Error(*Message.ToString(), this);
    }
}

bool UK2Node_ViewModelPropertyBase::IsActionFilteredOut(class FBlueprintActionFilter const& Filter)
{
    bool bFoundCompatible = false;

    // Prevent this node from showing up in Blueprints that do not have appropriate ViewModel
    for (UBlueprint* Blueprint : Filter.Context.Blueprints)
    {
        UClass* ViewClass = Blueprint->GeneratedClass;
        UClass* ViewModelClass = UnrealMvvm_Impl::FViewModelRegistry::GetViewModelClass(ViewClass);

        if (ViewModelClass && ViewModelOwnerClass->IsChildOf(ViewModelClass))
        {
            bFoundCompatible = true;
        }
        else
        {
            return true;
        }
    }

    return !bFoundCompatible;
}

FText UK2Node_ViewModelPropertyBase::GetMenuCategory() const
{
    return FEditorCategoryUtils::BuildCategoryString(FCommonEditorCategory::Variables, ViewModelOwnerClass->GetDisplayNameText());
}

void UK2Node_ViewModelPropertyBase::AddSpawners(FBlueprintActionDatabaseRegistrar& ActionRegistrar, TFunctionRef<bool(const UnrealMvvm_Impl::FViewModelPropertyReflection&)> PropertyFilter) const
{
    if (ActionRegistrar.IsOpenForRegistration(GetClass()))
    {
        for (const auto& PropertyPair : UnrealMvvm_Impl::FViewModelRegistry::GetAllProperties())
        {
            for (const auto& PropertyInfo : PropertyPair.Value)
            {
                if (!FViewModelPropertyNodeHelper::IsPropertyAvailableInBlueprint(PropertyInfo) || !PropertyFilter(PropertyInfo))
                {
                    continue;
                }

                UBlueprintNodeSpawner* Spawner = NewObject<UBlueprintNodeSpawner>();
                Spawner->NodeClass = GetClass();

                UClass* ViewModelClass = PropertyPair.Key;
                FName PropertyName = PropertyInfo.GetProperty()->GetName();

                //Spawner->DefaultMenuSignature.MenuName = FText::Format(MenuNameFormat, PropertyNameText);
                //Spawner->DefaultMenuSignature.Tooltip = FText::Format(MenuTooltipFormat, PropertyNameText);
                //Spawner->DefaultMenuSignature.Category = FEditorCategoryUtils::BuildCategoryString(FCommonEditorCategory::Variables, PropertyPair.Key->GetDisplayNameText());

                Spawner->CustomizeNodeDelegate.BindLambda([=](UEdGraphNode* Node, bool bIsTemplateNode)
                {
                    auto CastedNode = CastChecked<UK2Node_ViewModelPropertyBase>(Node);
                    CastedNode->ViewModelOwnerClass = ViewModelClass;
                    CastedNode->ViewModelPropertyName = PropertyName;
                });

                ActionRegistrar.AddBlueprintAction(GetClass(), Spawner);
            }
        }
    }
}