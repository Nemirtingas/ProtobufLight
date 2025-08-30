// Auto-generated from .proto
#pragma once

#include <ProtobufLight/ProtobufLightReflection.hpp>

enum class OneEnumLight : int32_t
{
    O_ZERO = 0,
    O_ONE = 1,
};

struct OneMsgLight
{
    int32_t id{};

    size_t GetByteSize() const { return ProtobufLight::Reflection::SerializedStructSize(*this); }
    bool ParseFromArray(const uint8_t* buffer, size_t size) { return ProtobufLight::Reflection::ParseStruct(*this, buffer, size); }
    std::string SerializeAsString() const { std::string out; ProtobufLight::Reflection::SerializeStruct(*this, out); return out; }
};

struct OneOfAllLight
{
    std::variant<std::monostate, int32_t, std::string, OneMsgLight, OneEnumLight> choice{ std::monostate{} };

    size_t GetByteSize() const { return ProtobufLight::Reflection::SerializedStructSize(*this); }
    bool ParseFromArray(const uint8_t* buffer, size_t size) { return ProtobufLight::Reflection::ParseStruct(*this, buffer, size); }
    std::string SerializeAsString() const { std::string out; ProtobufLight::Reflection::SerializeStruct(*this, out); return out; }
};

template<>
struct ProtobufLight::Reflection::ProtobufTrait<OneMsgLight>
{
    template<typename Obj, typename Callback>
    static void ForEachField(Obj& obj, Callback&& cb) {
        cb(obj.id, FieldMeta<1>{"id"});
    }
};

template<>
struct ProtobufLight::Reflection::ProtobufTrait<OneOfAllLight>
{
    template<typename Obj, typename Callback>
    static void ForEachField(Obj& obj, Callback&& cb) {
        cb(obj.choice, FieldMeta<1,3,4,5>{"choice"});
    }
};

