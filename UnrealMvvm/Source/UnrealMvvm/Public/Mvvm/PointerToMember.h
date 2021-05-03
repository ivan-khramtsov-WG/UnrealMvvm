// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

/* Helper struct for extracting Pointer to Member details */
template<typename T>
struct TPointerToMember;

/* Specialization for field members */
template<typename TValueType, typename TClassType>
struct TPointerToMember<TValueType TClassType::*>
{
    using ValueType = TValueType;
    using ClassType = TClassType;
};

/* Specialization for method members */
template<typename TValueType, typename TClassType, typename... TArgs>
struct TPointerToMember<TValueType (TClassType::*)(TArgs...)>
{
    using ValueType = TValueType;
    using ClassType = TClassType;
};