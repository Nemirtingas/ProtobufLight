// Auto-generated from .proto
#pragma once

#include <ProtobufLight/ProtobufLightReflection.hpp>

struct ValLight
{
    int32_t a{};
    std::string b{};

    size_t GetByteSize() const { return ProtobufLight::Reflection::SerializedStructSize(*this); }
    bool ParseFromArray(const uint8_t* buffer, size_t size) { return ProtobufLight::Reflection::ParseStruct(*this, buffer, size); }
    std::string SerializeAsString() const { std::string out; ProtobufLight::Reflection::SerializeStruct(*this, out); return out; }
};

struct InnerValLight
{
    ValLight v{};

    size_t GetByteSize() const { return ProtobufLight::Reflection::SerializedStructSize(*this); }
    bool ParseFromArray(const uint8_t* buffer, size_t size) { return ProtobufLight::Reflection::ParseStruct(*this, buffer, size); }
    std::string SerializeAsString() const { std::string out; ProtobufLight::Reflection::SerializeStruct(*this, out); return out; }
};

struct MapsMessagesLight
{
    std::map<std::string, ValLight> m_str_msg{};
    std::map<int32_t, ValLight> m_i32_msg{};
    std::map<int64_t, InnerValLight> m_i64_inner{};

    size_t GetByteSize() const { return ProtobufLight::Reflection::SerializedStructSize(*this); }
    bool ParseFromArray(const uint8_t* buffer, size_t size) { return ProtobufLight::Reflection::ParseStruct(*this, buffer, size); }
    std::string SerializeAsString() const { std::string out; ProtobufLight::Reflection::SerializeStruct(*this, out); return out; }
};

template<>
struct ProtobufLight::Reflection::ProtobufTrait<ValLight>
{
    template<typename Obj, typename Callback>
    static void ForEachField(Obj& obj, Callback&& cb) {
        cb(obj.a, FieldMeta<1>{"a"});
        cb(obj.b, FieldMeta<2>{"b"});
    }
};

template<>
struct ProtobufLight::Reflection::ProtobufTrait<InnerValLight>
{
    template<typename Obj, typename Callback>
    static void ForEachField(Obj& obj, Callback&& cb) {
        cb(obj.v, FieldMeta<1>{"v"});
    }
};

template<>
struct ProtobufLight::Reflection::ProtobufTrait<MapsMessagesLight>
{
    template<typename Obj, typename Callback>
    static void ForEachField(Obj& obj, Callback&& cb) {
        cb(obj.m_str_msg, FieldMeta<1>{"m_str_msg"});
        cb(obj.m_i32_msg, FieldMeta<2>{"m_i32_msg"});
        cb(obj.m_i64_inner, FieldMeta<3>{"m_i64_inner"});
    }
};

