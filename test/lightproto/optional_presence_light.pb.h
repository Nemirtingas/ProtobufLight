// Auto-generated from .proto
#pragma once

#include <ProtobufLight/ProtobufLightReflection.hpp>

enum class OptEnumLight : int32_t
{
    OPT_ZERO = 0,
    OPT_ONE = 1,
};

struct OptionalPresenceLight
{
    struct HasPresenceLight
    {
        int32_t x{};

        size_t GetByteSize() const { return ProtobufLight::Reflection::SerializedStructSize(*this); }
        bool ParseFromArray(const uint8_t* buffer, size_t size) { return ProtobufLight::Reflection::ParseStruct(*this, buffer, size); }
        std::string SerializeAsString() const { std::string out; ProtobufLight::Reflection::SerializeStruct(*this, out); return out; }
    };

    std::optional<int32_t> o_int32{};
    std::optional<std::string> o_string{};
    std::optional<OptEnumLight> o_enum{};
    HasPresenceLight msg{};

    size_t GetByteSize() const { return ProtobufLight::Reflection::SerializedStructSize(*this); }
    bool ParseFromArray(const uint8_t* buffer, size_t size) { return ProtobufLight::Reflection::ParseStruct(*this, buffer, size); }
    std::string SerializeAsString() const { std::string out; ProtobufLight::Reflection::SerializeStruct(*this, out); return out; }
};

template<>
struct ProtobufLight::Reflection::ProtobufTrait<OptionalPresenceLight>
{
    template<typename Obj, typename Callback>
    static void ForEachField(Obj& obj, Callback&& cb) {
        cb(obj.o_int32, FieldMeta<1>{"o_int32"});
        cb(obj.o_string, FieldMeta<2>{"o_string"});
        cb(obj.o_enum, FieldMeta<3>{"o_enum"});
        cb(obj.msg, FieldMeta<10>{"msg"});
    }
};

template<>
struct ProtobufLight::Reflection::ProtobufTrait<OptionalPresenceLight::HasPresenceLight>
{
    template<typename Obj, typename Callback>
    static void ForEachField(Obj& obj, Callback&& cb) {
        cb(obj.x, FieldMeta<1>{"x"});
    }
};

