/* Copyright (c) 2025 Nemiritngas
 * All rights reserved.
 *
 * Permission is granted to use, copy, and modify this software for personal or educational purposes only.
 * Commercial use, including but not limited to selling, licensing, or incorporating this software into a
 * commercial product, is strictly prohibited without the prior written consent of the author.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
 */

#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <array>
#include <variant>
#include <optional>
#include <type_traits>
#include <stdexcept>
#include <limits>
#include <cstdint>
#include <cstring>
#include <cassert>

namespace ProtobufLight {

struct ParseError : public std::runtime_error
{
    explicit ParseError(const std::string& m) : std::runtime_error(m) {}
    explicit ParseError(const char* m) : std::runtime_error(m) {}
};

namespace Detail {
    template<typename, typename = void>
    struct has_data_and_size : std::false_type {};

    template<typename T>
    struct has_data_and_size<T, std::void_t<
        decltype(std::declval<const T&>().data()),
        decltype(std::declval<const T&>().size())
    >> : std::true_type {};

    template<typename T>
    constexpr bool has_data_and_size_v = has_data_and_size<T>::value;

    template<typename T, typename... Ts>
    constexpr bool is_any_v = (std::is_same_v<T, Ts> || ...);

    template<typename T>
    constexpr bool is_byte_like_v = is_any_v<T, std::byte, uint8_t, unsigned char, int8_t, signed char, char>;

    template<typename T>
    struct is_variant : std::false_type {};

    template<typename... Ts>
    struct is_variant<std::variant<Ts...>> : std::true_type {};

    template<typename T>
    constexpr bool is_variant_v = is_variant<T>::value;

    template<typename C, typename = void>
    struct is_byte_container : std::false_type {};

    template<typename C>
    struct is_byte_container<C, std::enable_if_t<has_data_and_size_v<C>>> {
    private:
        using PtrT = decltype(std::declval<const C&>().data());
        using ElemT = std::remove_cv_t<std::remove_pointer_t<PtrT>>;
    public:
        static constexpr bool value = is_byte_like_v<ElemT>;
    };

    template<typename C>
    constexpr bool is_byte_container_v = is_byte_container<C>::value;

    template<typename, typename = void>
    struct has_push_back_method : std::false_type {};

    template<typename C>
    struct has_push_back_method<C, std::void_t<
        decltype(std::declval<C&>().push_back(std::declval<typename C::value_type>()))
    >> : std::true_type {};

    template<typename C>
    constexpr bool has_push_back_method_v = has_push_back_method<C>::value;

    template<typename C>
    constexpr bool is_appendable_byte_container_v = is_byte_container_v<C> && has_push_back_method_v<C>;

    template<typename T>
    struct is_std_map : std::false_type {};

    template<typename K, typename V, typename Pr, typename Alloc>
    struct is_std_map<std::map<K, V, Pr, Alloc>> : std::true_type {};

    template<typename T>
    constexpr bool is_std_map_v = is_std_map<T>::value;

    template<typename T>
    struct is_std_vector : std::false_type {};

    template<typename T>
    struct is_std_vector<std::vector<T>> : std::true_type {};

    template<typename T>
    constexpr bool is_std_vector_v = is_std_vector<T>::value;

    template<typename T>
    struct is_std_optional : std::false_type {};

    template<typename T>
    struct is_std_optional<std::optional<T>> : std::true_type {};

    template<typename T>
    constexpr bool is_std_optional_v = is_std_optional<T>::value;

    template<typename MemberT, typename MetaT>
    constexpr void ValidateFieldmetaVariant() {
        static_assert(is_variant_v<MemberT>, "Called validate_fieldmeta_variant on non-variant");

        using FirstAlt = std::variant_alternative_t<0, MemberT>;
        static_assert(std::is_same_v<FirstAlt, std::monostate>,
            "Oneof variant must have std::monostate as first alternative");

        constexpr size_t variant_size = std::variant_size_v<MemberT>;
        constexpr size_t expected_numbers = variant_size - 1;

        constexpr auto& nums = MetaT::numbers;
        static_assert(nums.size() == expected_numbers,
            "FieldMeta numbers must have exactly variant_size-1 entries for oneof (excluding monostate)");
    }

} // namespace Detail

template<typename T, typename Variant>
T& EnsureVariant(Variant& v, typename std::enable_if_t<!std::is_same_v<T, std::monostate>, int> = 0)
{
    if (!std::holds_alternative<T>(v))
    {
        v = T{};
    }
    return std::get<T>(v);
}

// WireType protobuf
enum WireType : uint8_t {
    VARINT = 0,
    FIXED64 = 1,
    LENGTH_DELIMITED = 2,
    FIXED32 = 5,
};

constexpr uint32_t EncodeZigzag32(int32_t n) noexcept {
    return (static_cast<uint32_t>(n) << 1) ^ static_cast<uint32_t>(n >> 31);
}

constexpr int32_t DecodeZigzag32(uint32_t v) noexcept {
    return static_cast<int32_t>((v >> 1) ^ static_cast<uint32_t>(-static_cast<int32_t>(v & 1)));
}

constexpr uint64_t EncodeZigzag64(int64_t n) noexcept {
    return (static_cast<uint64_t>(n) << 1) ^ static_cast<uint64_t>(n >> 63);
}

constexpr int64_t DecodeZigzag64(uint64_t v) noexcept {
    return static_cast<int64_t>((v >> 1) ^ static_cast<uint64_t>(-static_cast<int64_t>(v & 1)));
}

template <typename T>
constexpr auto EncodeZigzag(T n) noexcept
{
    static_assert(std::is_signed_v<T>, "ZigZag encoding only for signed types");
    if constexpr (sizeof(T) == 4)
    {
        return EncodeZigzag32(static_cast<int32_t>(n));
    }
    else if constexpr (sizeof(T) == 8)
    {
        return EncodeZigzag64(static_cast<int64_t>(n));
    }
    else
    {
        // fallback generic: promote to 64-bit then cast back
        using Promoted = std::conditional_t<(sizeof(T) < 8), int64_t, T>;
        auto encoded = EncodeZigzag64(static_cast<int64_t>(n));
        return static_cast<std::make_unsigned_t<T>>(encoded);
    }
}

template <typename U>
constexpr auto DecodeZigzag(U v) noexcept
{
    static_assert(std::is_unsigned_v<U>, "ZigZag decoding only for unsigned types");
    if constexpr (sizeof(U) == 4)
    {
        return DecodeZigzag32(static_cast<uint32_t>(v));
    }
    else if constexpr (sizeof(U) == 8)
    {
        return DecodeZigzag64(static_cast<uint64_t>(v));
    }
    else
    {
        // fallback generic: promote to 64-bit then cast back
        auto decoded = DecodeZigzag64(static_cast<uint64_t>(v));
        using Target = std::make_signed_t<U>;
        return static_cast<Target>(decoded);
    }
}

inline constexpr uint64_t MakeTag(int fieldNumber, WireType wire_type)
{
    return (static_cast<uint64_t>(fieldNumber) << 3) | static_cast<uint64_t>(wire_type);
}

inline size_t VarintEncodedSize(uint64_t value)
{
    size_t size = 0;
    while (value >= 0x80)
    {
        ++size;
        value >>= 7;
    }
    ++size;
    return size;
}

template<typename Container, typename = std::enable_if_t<Detail::is_appendable_byte_container_v<Container>>>
void EncodeVarint(uint64_t value, Container& out)
{
    while (value >= 0x80)
    {
        out.push_back(static_cast<typename Container::value_type>((value & 0x7F) | 0x80));
        value >>= 7;
    }
    out.push_back(static_cast<typename Container::value_type>(value));
}

inline bool DecodeVarint(const uint8_t* buf, size_t size, size_t& idx, uint64_t& value)
{
    uint64_t result = 0;
    int shift = 0;
    while (idx < size)
    {
        uint8_t byte = buf[idx++];
        result |= uint64_t(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0)
        {
            value = result;
            return true;
        }
        shift += 7;
        if (shift > 63)
            return false;
    }
    return false;
}

template<typename Container>
std::enable_if_t<Detail::is_appendable_byte_container_v<Container>> WriteKey(uint32_t fieldNumber, uint8_t wireType, Container& out)
{
    EncodeVarint((fieldNumber << 3) | wireType, out);
}

inline bool ReadKey(const uint8_t* buf, size_t size, size_t& idx, uint32_t& fieldNumber, uint8_t& wireType)
{
    uint64_t key;
    if (!DecodeVarint(buf, size, idx, key))
        return false;

    fieldNumber = static_cast<uint32_t>(key >> 3);
    wireType = static_cast<uint8_t>(key & 0x7);
    return true;
}

template<typename T, typename Container>
std::enable_if_t<Detail::is_appendable_byte_container_v<Container>> Write(T&& value, Container& out)
{
    using DecayT = std::decay_t<T>;
    if constexpr (Detail::is_any_v<DecayT, int32_t, int64_t, uint32_t, uint64_t, size_t>)
    {
        EncodeVarint(value, out);
    }
    else if constexpr (std::is_same_v<DecayT, bool>)
    {
        EncodeVarint(value == false ? 0 : 1, out);
    }
    else if constexpr (std::is_enum_v<DecayT>)
    {
        using U = std::underlying_type_t<DecayT>;
        EncodeVarint(static_cast<U>(value), out);
    }
    else if constexpr (Detail::is_any_v<DecayT, float, double>)
    {
        out.insert(out.end(), reinterpret_cast<const typename Container::value_type*>(&value), reinterpret_cast<const typename Container::value_type*>(&value) + sizeof(DecayT));
    }
    else if constexpr (Detail::is_byte_container_v<DecayT>)
    {
        EncodeVarint(value.size(), out);
        out.insert(out.end(), reinterpret_cast<const typename Container::value_type*>(value.data()), reinterpret_cast<const typename Container::value_type*>(value.data()) + value.size());
    }
    else
    {
        static_assert(sizeof(T) == 0, "Unsupported field type");
    }
}

template<typename T>
size_t SerializedSize(T&& value)
{
    using DecayT = std::decay_t<T>;
    if constexpr (Detail::is_any_v<DecayT, int32_t, int64_t, uint32_t, uint64_t, size_t>)
    {
        return VarintEncodedSize(value);
    }
    else if constexpr (std::is_same_v<DecayT, bool>)
    {
        return VarintEncodedSize(value == false ? 0 : 1);
    }
    else if constexpr (std::is_enum_v<DecayT>)
    {
        using U = std::underlying_type_t<DecayT>;
        return VarintEncodedSize(static_cast<U>(value));
    }
    else if constexpr (Detail::is_any_v<DecayT, float, double>)
    {
        return sizeof(DecayT);
    }
    else if constexpr (Detail::is_byte_container_v<DecayT>)
    {
        return VarintEncodedSize(value.size()) + value.size();
    }
    else
    {
        static_assert(sizeof(T) == 0, "Unsupported field type");
    }
}

inline bool SkipField(uint8_t wireType, const uint8_t* buf, size_t size, size_t& idx)
{
    switch (wireType) {
        case WireType::VARINT:
        {
            uint64_t tmp;
            if (!DecodeVarint(buf, size, idx, tmp))
                return false;
            break;
        }
        case WireType::FIXED64:
            if (idx + 8 > size)
                return false;

            idx += 8;
            break;
        case WireType::LENGTH_DELIMITED: {
            uint64_t len;
            if (!DecodeVarint(buf, size, idx, len))
                return false;

            if (idx + len > size)
                return false;

            idx += static_cast<size_t>(len);
            break;
        }
        case WireType::FIXED32:
            if (idx + 4 > size)
                return false;

            idx += 4;
            break;
        default:
            return false;
    }

    return true;
}

template<typename T>
bool Read(const uint8_t* buf, size_t size, size_t& idx, T& value)
{
    using DecayT = std::decay_t<T>;
    if (idx >= size)
        return false;

    if constexpr (Detail::is_any_v<DecayT, int32_t, int64_t, uint32_t, uint64_t, size_t>)
    {
        uint64_t tmp;
        if (!DecodeVarint(buf, size, idx, tmp))
            return false;

        value = static_cast<DecayT>(tmp);
    }
    else if constexpr (std::is_same_v<DecayT, bool>)
    {
        uint64_t tmp;
        if (!DecodeVarint(buf, size, idx, tmp))
            return false;
        
        value = tmp == 1;
    }
    else if constexpr (std::is_enum_v<DecayT>)
    {
        using U = std::underlying_type_t<DecayT>;
        uint64_t tmp;
        if (!DecodeVarint(buf, size, idx, tmp))
            return false;

        value = static_cast<DecayT>(static_cast<U>(tmp));
    }
    else if constexpr (Detail::is_any_v<DecayT, float, double>)
    {
        if ((idx + sizeof(DecayT)) > size)
            return false;

        std::memcpy(reinterpret_cast<void*>(&value), reinterpret_cast<const void*>(buf + idx), sizeof(DecayT));
        idx += sizeof(DecayT);
    }
    else if constexpr (std::is_same_v<DecayT, std::string_view>)
    {
        uint64_t length;
        if (!DecodeVarint(buf, size, idx, length))
            return false;

        if ((idx + length) > size)
            return false;

        value = std::string_view{ reinterpret_cast<const char*>(buf) + idx, static_cast<std::string_view::size_type>(length) };

        idx += length;
    }
    else if constexpr (Detail::is_std_optional_v<DecayT>)
    {
        typename DecayT::value_type optionalValue;
        if (!Read(buf, size, idx, optionalValue))
            return false;
        value.emplace(std::move(optionalValue));
    }
    else if constexpr (Detail::is_appendable_byte_container_v<DecayT>)
    {
        uint64_t length;
        if (!DecodeVarint(buf, size, idx, length))
            return false;

        if ((idx + length) > size)
            return false;

        value.insert(
            value.end(),
            reinterpret_cast<const typename DecayT::value_type*>(buf) + idx,
            reinterpret_cast<const typename DecayT::value_type*>(buf) + idx + length);

        idx += length;
    }
    else
    {
        static_assert(sizeof(T) == 0, "Unsupported field type");
    }

    return true;
}

} // namespace ProtobufLight
