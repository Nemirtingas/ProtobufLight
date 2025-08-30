/* Copyright (c) 2025 Nemiritngas
 * All rights reserved.
 *
 * Permission is granted to use, copy, and modify this software for personal or educational purposes only.
 * Commercial use, including but not limited to selling, licensing, or incorporating this software into a
 * commercial product, is strictly prohibited without the prior written consent of the author.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
 */

#include "ProtobufLight.hpp" 

#pragma once

namespace ProtobufLight {
namespace Reflection {

// Forward declaration
template<typename T>
bool ParseStruct(T& obj, const uint8_t* buf, size_t size);

template<typename T, typename Container>
std::enable_if_t<ProtobufLight::Detail::is_appendable_byte_container_v<Container>> SerializeStruct(const T& obj, Container& out);

template<typename T>
size_t SerializedStructSize(const T& obj);

template<typename T>
struct ProtobufTrait {
    //template<typename Obj, typename Callback>
    //static void ForEachField(Obj& obj, Callback&& cb) {
    //    static_assert(false, "You must implement 'ForEachField' in your own 'ProtobufTrait<T>' trait.");
    //}
};

namespace Detail {

    struct AnyFieldVisitor {
        template <typename... Args>
        void operator()(Args&&...) const noexcept {}
    };

    template <typename T>
    auto has_trait_impl(int) -> decltype(
        ProtobufTrait<T>::ForEachField(
            std::declval<T&>(),
            AnyFieldVisitor{}
        ),
        std::true_type{}
    );

    template <typename>
    std::false_type has_trait_impl(...);

    template <typename T>
    struct has_protobuf_trait : decltype(Detail::has_trait_impl<T>(0)) {};

    template <typename T>
    constexpr bool has_protobuf_trait_v = has_protobuf_trait<T>::value;

    template <typename Alt>
    bool TryParseVariantAlternative(uint8_t wireType,
                                       const uint8_t* buf, size_t size, size_t& idx,
                                       Alt& out)
    {
        if constexpr (std::is_same_v<Alt, std::monostate>) {
            return false;
        } else if constexpr (has_protobuf_trait_v<Alt>) {
            std::string_view innerBuf;
            if (!Read(buf, size, idx, innerBuf))
                return false;

            if (!ParseStruct(out, reinterpret_cast<const uint8_t*>(innerBuf.data()), innerBuf.size()))
                return false;

        } else if (!Read(buf, size, idx, out)) {
            return false;
        }
        return true;
    }

    template <typename Variant, size_t... Is>
    bool ParseOneofImpl(Variant& member,
                          const std::array<int, std::variant_size_v<Variant> - 1>& nums,
                          uint32_t fieldNumber,
                          uint8_t wireType,
                          const uint8_t* buf,
                          size_t size,
                          size_t& idx,
                          std::index_sequence<Is...>)
    {
        bool handled = false;

        // Creates 1 lambda for each 'Is' in the std::index_sequence and calls it '()'

        (([&]{
            if (handled) return;
            // Current sequence index
            constexpr size_t I = Is;
            using Alt = std::variant_alternative_t<I, Variant>;
            if constexpr (std::is_same_v<Alt, std::monostate>)
            {
                return;
            }
            else
            {
                const size_t nums_idx = (I == 0 ? std::numeric_limits<size_t>::max() : I - 1);
                if (nums_idx >= nums.size())
                    return;

                if (static_cast<uint32_t>(nums[nums_idx]) != fieldNumber)
                    return;

                Alt tmp{};
                const size_t before = idx;
                if (!TryParseVariantAlternative<Alt>(wireType, buf, size, idx, tmp))
                {
                    idx = before;
                    return;
                }

                member = std::move(tmp);
                handled = true;
            }
        }()), ...);

        return handled;
    }

    template <typename Variant>
    bool ParseOneof(Variant& member,
                     const std::array<int, std::variant_size_v<Variant> - 1>& nums,
                     uint32_t fieldNumber,
                     uint8_t wireType,
                     const uint8_t* buf,
                     size_t size,
                     size_t& idx)
    {
        return ParseOneofImpl(member, nums, fieldNumber, wireType, buf, size, idx,
                                std::make_index_sequence<std::variant_size_v<Variant>>{});
    }

} // namespace Detail

template<int... Nums>
struct FieldMeta {
    constexpr FieldMeta(std::string_view name_) : name(name_) {}
    std::string_view name;
    static constexpr std::array<int, sizeof...(Nums)> numbers = { Nums... };
};
template<int... Nums>
constexpr std::array<int, sizeof...(Nums)> FieldMeta<Nums...>::numbers;


template<typename T>
size_t SerializedFieldSize(uint32_t fieldNumber, T&& value, bool isVariant)
{
    using DecayT = std::decay_t<T>;

    size_t serializedSize = 0;

    if constexpr (ProtobufLight::Detail::is_any_v<DecayT, int32_t, int64_t>)
    {
        if (isVariant || value != DecayT{})
        {
            // Key
            serializedSize += 1;
            // TODO: Choose to encode zigzag or not
            if (true)
                // Data
                serializedSize += SerializedSize(value);
            else
                // Data
                serializedSize += SerializedSize(EncodeZigzag(value));
        }
    }
    else if constexpr (ProtobufLight::Detail::is_any_v<DecayT, uint32_t, uint64_t, size_t, bool> ||
                       std::is_enum_v<DecayT>)
    {
        if (isVariant || value != DecayT{})
        {
            // Key // Data
            serializedSize += 1 + SerializedSize(value);
        }
    }
    else if constexpr (ProtobufLight::Detail::is_any_v<DecayT, float, double>)
    {
        if (isVariant || value != DecayT{})
        {
            // Key // Data
            serializedSize += 1 + SerializedSize(value);
        }
    }
    else if constexpr (ProtobufLight::Reflection::Detail::has_protobuf_trait_v<DecayT>)
    {
        const size_t innerSerializedSize = SerializedStructSize(value);
        // Key // Length // Data
        serializedSize += 1 + SerializedSize(innerSerializedSize) + innerSerializedSize;
    }
    else if constexpr (ProtobufLight::Detail::is_std_optional_v<DecayT>)
    {
        if (value.has_value())
        {
            // Key // Data
            serializedSize += 1 + SerializedFieldSize(fieldNumber, value.value(), false);
        }
    }
    else if constexpr (ProtobufLight::Detail::is_std_vector_v<DecayT>)
    {
        using DecayItemT = std::decay_t<typename DecayT::value_type>;

        if (!isVariant && value.empty())
            return serializedSize;

        if constexpr (ProtobufLight::Detail::is_std_map_v<DecayItemT> ||
            ProtobufLight::Detail::is_std_optional_v<DecayItemT>)
        {
            static_assert(sizeof(T) == 0, "Unsupported repeated field type");
        }
        // Message are serialized as multiple messages with the same fieldNumber
        else if constexpr (ProtobufLight::Reflection::Detail::has_protobuf_trait_v<DecayItemT> ||
            ProtobufLight::Detail::is_byte_container_v<DecayItemT>)
        {
            // SerializedFieldSize already has the key
            for (auto&& item : value)
                serializedSize += SerializedFieldSize(fieldNumber, item, false);
        }
        // Scalar is serialized as a message pack.
        else
        {
            size_t repeatedLength = 0;
            for (auto&& item : value)
                repeatedLength += SerializedSize(item);

            // Key // Length // Data
            serializedSize += 1 + SerializedSize(repeatedLength) + repeatedLength;
        }
    }
    else if constexpr (ProtobufLight::Detail::is_std_map_v<DecayT>)
    {
        // protobuf map is an anonymous message like:
        // message map {
        //   Key key : 1;
        //   Value value : 2;
        // }
        for (const auto& [k, v] : value)
        {
            const size_t mapItemSize = SerializedFieldSize(1, k, false) + SerializedFieldSize(2, v, false);
            // Key // Length // Data
            serializedSize += 1 + SerializedSize(mapItemSize) + mapItemSize;
        }
    }
    else if constexpr (ProtobufLight::Detail::is_byte_container_v<DecayT>)
    {
        if (isVariant || !value.empty())
        {
            // Key // Length // Data
            serializedSize += 1 + SerializedSize(value.size()) + value.size();
        }
    }
    else
    {
        static_assert(sizeof(T) == 0, "Unsupported field type");
    }

    return serializedSize;
}

template<typename T>
size_t SerializedStructSize(const T& obj)
{
    size_t serializedSize = 0;
    ProtobufTrait<T>::ForEachField(const_cast<T&>(obj), [&](auto&& member, auto&& meta)
    {
        using MemberT = std::decay_t<decltype(member)>;
        using MetaT = std::decay_t<decltype(meta)>;
        constexpr auto& nums = MetaT::numbers;
        
        if constexpr (!ProtobufLight::Detail::is_variant_v<MemberT>)
        {
            static_assert(!nums.empty(), "FieldMeta must have at least one field number");
            static_assert(nums.size() == 1, "Non-variant field must have exactly one field number in FieldMeta");
            serializedSize += SerializedFieldSize(static_cast<uint32_t>(nums[0]), member, false);
        }
        else
        {
            ProtobufLight::Detail::ValidateFieldmetaVariant<MemberT, MetaT>();

            serializedSize += std::visit([&](auto&& v)
            {
                constexpr auto& nums = MetaT::numbers;
                using V = std::decay_t<decltype(v)>;
                if constexpr (!std::is_same_v<V, std::monostate>)
                    return SerializedFieldSize(nums[member.index() - 1], v, true);

                return size_t(0);
            }, member);
        }
    });

    return serializedSize;
}

template<
    typename T,
    typename Container>
std::enable_if_t<ProtobufLight::Detail::is_appendable_byte_container_v<Container>> SerializeField(uint32_t fieldNumber, T&& value, Container& out, bool isVariant)
{
    using DecayT = std::decay_t<T>;

    if constexpr (ProtobufLight::Detail::is_any_v<DecayT, int32_t, int64_t>)
    {
        if (isVariant || value != DecayT{})
        {
            WriteKey(fieldNumber, WireType::VARINT, out);
            // TODO: Choose to encode zigzag or not
            if (true)
                Write(value, out);
            else
                Write(EncodeZigzag(value), out);
        }
    }
    else if constexpr (ProtobufLight::Detail::is_any_v<DecayT, uint32_t, uint64_t, size_t, bool> || 
                       std::is_enum_v<DecayT>)
    {
        if (isVariant || value != DecayT{})
        {
            WriteKey(fieldNumber, WireType::VARINT, out);
            Write(value, out);
        }
    }
    else if constexpr (std::is_same_v<DecayT, float>)
    {
        if (isVariant || value != DecayT{})
        {
            WriteKey(fieldNumber, WireType::FIXED32, out);
            Write(value, out);
        }
    }
    else if constexpr (std::is_same_v<DecayT, double>)
    {
        if (isVariant || value != DecayT{})
        {
            WriteKey(fieldNumber, WireType::FIXED64, out);
            Write(value, out);
        }
    }
    else if constexpr (ProtobufLight::Reflection::Detail::has_protobuf_trait_v<DecayT>)
    {
        auto innerSize = SerializedStructSize(value);

        if (innerSize > 0)
        {
            WriteKey(fieldNumber, WireType::LENGTH_DELIMITED, out);
            Write(innerSize, out);
            const auto debugSize = out.size();

            if (innerSize > 0)
                SerializeStruct(value, out);

            assert(debugSize + innerSize == out.size() && "Serialized size doesn't match expected serialized size");
        }
    }
    else if constexpr (ProtobufLight::Detail::is_std_optional_v<DecayT>)
    {
        if (value.has_value())
            SerializeField(fieldNumber, value.value(), out, true);
    }
    else if constexpr (ProtobufLight::Detail::is_std_vector_v<DecayT>)
    {
        using DecayItemT = std::decay_t<typename DecayT::value_type>;

        if (!isVariant && value.empty())
            return;

        if constexpr (ProtobufLight::Detail::is_std_map_v<DecayItemT> ||
                      ProtobufLight::Detail::is_std_optional_v<DecayItemT>)
        {
            static_assert(sizeof(T) == 0, "Unsupported repeated field type");
        }
        // Message are serialized as multiple messages with the same fieldNumber
        else if constexpr (ProtobufLight::Reflection::Detail::has_protobuf_trait_v<DecayItemT> ||
            ProtobufLight::Detail::is_byte_container_v<DecayItemT>)
        {
            for (auto&& item : value)
                SerializeField(fieldNumber, item, out, false);
        }
        // Scalar is serialized as a message pack.
        else
        {
            size_t repeatedLength = 0;
            for (auto&& item : value)
                repeatedLength += SerializedSize(item);

            WriteKey(fieldNumber, WireType::LENGTH_DELIMITED, out);
            Write(repeatedLength, out);
            for (auto&& item : value)
                Write(item, out);
        }
    }
    else if constexpr (ProtobufLight::Detail::is_std_map_v<DecayT>)
    {
        // protobuf map is an anonymous message like:
        // message map {
        //   Key key : 1;
        //   Value value : 2;
        // }
        for (const auto& [k, v] : value)
        {
            auto mapEntrySize = SerializedFieldSize(1, k, false) + SerializedFieldSize(2, v, false);

            WriteKey(fieldNumber, WireType::LENGTH_DELIMITED, out);
            Write(mapEntrySize, out);
            SerializeField(1, k, out, false);
            SerializeField(2, v, out, false);
        }
    }
    else if constexpr (ProtobufLight::Detail::is_byte_container_v<DecayT>)
    {
        if (!isVariant && value.empty())
            return;

        WriteKey(fieldNumber, WireType::LENGTH_DELIMITED, out);
        Write(value, out);
    }
    else
    {
        static_assert(sizeof(T) == 0, "Unsupported field type");
    }
}

template<typename T>
bool ParseField(uint32_t fieldNumber, uint8_t wireType, const uint8_t* buf, size_t size, size_t& idx, T& value)
{
    using DecayT = std::decay_t<T>;

    if (idx >= size)
    {
        value = DecayT{};
        return true;
    }

    if constexpr (ProtobufLight::Detail::is_any_v<DecayT, int32_t, int64_t, uint32_t, uint64_t, size_t, bool>)
    {
        if (wireType != WireType::VARINT)
            return false;

        // TODO: Choose to encode zigzag or not
        return Read(buf, size, idx, value);
    }
    else if constexpr (std::is_enum_v<DecayT>)
    {
        if (wireType != WireType::VARINT)
            return false;

        return Read(buf, size, idx, value);
    }
    else if constexpr (std::is_same_v<DecayT, float>)
    {
        if (wireType != WireType::FIXED32)
            return false;

        return Read(buf, size, idx, value);
    }
    else if constexpr (std::is_same_v<DecayT, double>)
    {
        if (wireType != WireType::FIXED64)
            return false;

        return Read(buf, size, idx, value);
    }
    else if constexpr (ProtobufLight::Reflection::Detail::has_protobuf_trait_v<DecayT>)
    {
        std::string_view innerBuf;
        if (!Read(buf, size, idx, innerBuf))
            return false;

        return ParseStruct(value, reinterpret_cast<const uint8_t*>(innerBuf.data()), innerBuf.size());
    }
    else if constexpr (ProtobufLight::Detail::is_std_optional_v<DecayT>)
    {
        return Read(buf, size, idx, value);
    }
    else if constexpr (ProtobufLight::Detail::is_std_vector_v<DecayT>)
    {
        using ElemT = typename DecayT::value_type;
        std::string_view innerBuf;
        if (!Read(buf, size, idx, innerBuf))
            return false;
        
        if constexpr (ProtobufLight::Reflection::Detail::has_protobuf_trait_v<ElemT>)
        {
            auto& v = value.emplace_back(ElemT{});
            return ParseStruct(v, reinterpret_cast<const uint8_t*>(innerBuf.data()), innerBuf.size());
        }
        else if constexpr (ProtobufLight::Detail::is_byte_container_v<ElemT>)
        {
            auto& v = value.emplace_back(innerBuf.begin(), innerBuf.end());
            return true;
        }
        else
        {
            size_t innerIdx = 0;
            while (innerIdx < innerBuf.length())
            {
                auto& v = value.emplace_back(ElemT{});
                if (!Read(reinterpret_cast<const uint8_t*>(innerBuf.data()), innerBuf.size(), innerIdx, v))
                    return false;
            }
            return true;
        }
    }
    else if constexpr (ProtobufLight::Detail::is_std_map_v<DecayT>)
    {
        typename DecayT::key_type key{};
        typename DecayT::mapped_type v{};
        size_t innerIdx = 0;
        std::string_view innerBuf;
        if (!Read(buf, size, idx, innerBuf))
            return false;

        uint32_t innerFieldNumber;
        uint8_t innerWireType;

        for (int i = 1; i < 3 && innerIdx < innerBuf.size(); ++i)
        {
            if (!ReadKey(reinterpret_cast<const uint8_t*>(innerBuf.data()), innerBuf.size(), innerIdx, innerFieldNumber, innerWireType))
                return false;

            if (innerFieldNumber == 1)
            {
                if (!ParseField(innerFieldNumber, innerWireType, reinterpret_cast<const uint8_t*>(innerBuf.data()), innerBuf.size(), innerIdx, key))
                    return false;
            }
            else if (innerFieldNumber == 2)
            {
                if (!ParseField(innerFieldNumber, innerWireType, reinterpret_cast<const uint8_t*>(innerBuf.data()), innerBuf.size(), innerIdx, v))
                    return false;
            }
            else
            {
                return false;
            }
        }

        value.emplace(std::move(key), std::move(v));
        return true;
    }
    else if constexpr (ProtobufLight::Detail::is_appendable_byte_container_v<DecayT>)
    {
        if (wireType != WireType::LENGTH_DELIMITED)
            return false;

        return Read(buf, size, idx, value);
    }
    else
    {
        static_assert(sizeof(T) == 0, "Unsupported field type");
    }

    return false;
}

template<typename T, typename Container>
std::enable_if_t<ProtobufLight::Detail::is_appendable_byte_container_v<Container>> SerializeStruct(const T& obj, Container& out)
{
    ProtobufTrait<T>::ForEachField(const_cast<T&>(obj), [&](auto&& member, auto&& meta)
    {
        using MemberT = std::decay_t<decltype(member)>;
        using MetaT = std::decay_t<decltype(meta)>;
        constexpr auto& nums = MetaT::numbers;
        
        if constexpr (!ProtobufLight::Detail::is_variant_v<MemberT>)
        {
            static_assert(!nums.empty(), "FieldMeta must have at least one field number");
            static_assert(nums.size() == 1, "Non-variant field must have exactly one field number in FieldMeta");
            SerializeField(static_cast<uint32_t>(nums[0]), member, out, false);
        }
        else
        {
            ProtobufLight::Detail::ValidateFieldmetaVariant<MemberT, MetaT>();

            std::visit([&](auto&& v)
            {
                using V = std::decay_t<decltype(v)>;
                if constexpr (!std::is_same_v<V, std::monostate>)
                {
                    constexpr auto& nums = MetaT::numbers;
                    size_t active = member.index();
                    if (active == 0)
                        return;

                    uint32_t field_number = static_cast<uint32_t>(nums[active - 1]);
                    SerializeField(field_number, v, out, true);
                }
            }, member);
        }
    });
}

template<typename T>
bool ParseStruct(T& obj, const uint8_t* buf, size_t size)
{
    size_t idx = 0;
    auto result = true;

    obj = T{};
    while (idx < size)
    {
        uint32_t fieldNumber;
        uint8_t wireType;
        if (!ReadKey(buf, size, idx, fieldNumber, wireType))
            return false;

        bool fieldHandled = false;

        ProtobufTrait<T>::ForEachField(obj, [&](auto&& member, auto&& meta)
        {
            using MemberT = std::decay_t<decltype(member)>;
            using MetaT = std::decay_t<decltype(meta)>;

            if (fieldHandled && !ProtobufLight::Detail::is_std_vector_v<MemberT>)
                return;

            constexpr auto& nums = MetaT::numbers;
            const size_t idxBackup = idx;

            // Vérifie si fieldNumber correspond à un des numéros de ce champ
            for (auto n : nums)
            {
                if (n == fieldNumber)
                {
                    if constexpr (!ProtobufLight::Detail::is_variant_v<MemberT>)
                    {
                        if (!ParseField(fieldNumber, wireType, buf, size, idx, member))
                        {
                            idx = idxBackup;
                            result = false;
                        }
                    }
                    else
                    {
                        if (!Detail::ParseOneof(member, nums, fieldNumber, wireType, buf, size, idx))
                        {
                            member = std::monostate{};
                        }
                    }
                    fieldHandled = true;
                    break;
                }
            }
        });

        if (!fieldHandled)
        {
            if (!SkipField(wireType, buf, size, idx))
                return false;
        }
    }

    return result;
}

} // namespace Reflection
} // namespace ProtobufLight
