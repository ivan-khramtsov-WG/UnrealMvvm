// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/Impl/ValueTypeTraits.h"
#include "Mvvm/Impl/PropertyFactory.h"

template<typename TOwner, typename TValue>
class TViewModelProperty;

class FViewModelPropertyBase;
class UBaseViewModel;

namespace UnrealMvvm_Impl
{

    namespace Details
    {
        /* Helper class to provide GetCastedProperty method */
        template <typename TOwner, typename TValue>
        struct TBaseOperation : public FViewModelPropertyOperations
        {
            using TDecayedValue = typename TDecay<TValue>::Type;

            const TViewModelProperty<TOwner, TValue>* GetCastedProperty() const
            {
                return static_cast<const TViewModelProperty<TOwner, TValue>*>(Property);
            }
        };

        /* Implementation of CopyValue method */
        template <typename TBaseOp, typename TOwner, typename TValue, bool bOptional>
        struct TCopyValueOperation;

        template <typename TBaseOp, typename TOwner, typename TValue>
        struct TCopyValueOperation<TBaseOp, TOwner, TValue, true> : public TBaseOp
        {
            void CopyValue(UBaseViewModel* InViewModel, void* OutValue, bool& OutHasValue) const override
            {
                check(InViewModel);

                TValue Value = this->GetCastedProperty()->GetValue((TOwner*)InViewModel);
                OutHasValue = Value.IsSet();
                if (OutHasValue)
                {
                    *((typename TBaseOp::TDecayedValue*)OutValue) = Value.GetValue();
                }
            }
        };

        template <typename TBaseOp, typename TOwner, typename TValue>
        struct TCopyValueOperation<TBaseOp, TOwner, TValue, false> : public TBaseOp
        {
            void CopyValue(UBaseViewModel* InViewModel, void* OutValue, bool& OutHasValue) const override
            {
                check(InViewModel);

                *((typename TBaseOp::TDecayedValue*)OutValue) = this->GetCastedProperty()->GetValue((TOwner*)InViewModel);
                OutHasValue = true;
            }
        };

        /* Implementation of AddClassProperty method */
        template <typename TBaseOp, typename TOwner, typename TValue>
        struct TAddClassPropertyOperation : public TBaseOp
        {
            void AddClassProperty(UClass* TargetClass) const override
            {
                const TViewModelProperty<TOwner, TValue>* Prop = this->GetCastedProperty();
                if (Prop->GetFieldOffset() > 0)
                {
                    TPropertyFactory<typename TDecay<TValue>::Type>::AddProperty(TargetClass, Prop->GetFieldOffset(), Prop->GetName());
                }
            }
        };
    }

    template <typename TBaseOp>
    struct TViewModelPropertyOperations : public TBaseOp
    {
        TViewModelPropertyOperations(const FViewModelPropertyBase* Prop)
        {
            check(Prop);
            this->Property = Prop;
        }
    };

}