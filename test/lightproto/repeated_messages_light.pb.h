// Auto-generated from .proto
#pragma once

#include <ProtobufLight/ProtobufLightReflection.hpp>

struct ItemLight
{
    int32_t id{};
    std::string name{};

    size_t GetByteSize() const { return ProtobufLight::Reflection::SerializedStructSize(*this); }
    bool ParseFromArray(const uint8_t* buffer, size_t size) { return ProtobufLight::Reflection::ParseStruct(*this, buffer, size); }
    std::string SerializeAsString() const { std::string out; ProtobufLight::Reflection::SerializeStruct(*this, out); return out; }
};

struct WrapperLight
{
    ItemLight single{};
    std::vector<ItemLight> many{};

    size_t GetByteSize() const { return ProtobufLight::Reflection::SerializedStructSize(*this); }
    bool ParseFromArray(const uint8_t* buffer, size_t size) { return ProtobufLight::Reflection::ParseStruct(*this, buffer, size); }
    std::string SerializeAsString() const { std::string out; ProtobufLight::Reflection::SerializeStruct(*this, out); return out; }
};

struct RepeatedMessagesLight
{
    std::vector<ItemLight> items{};
    std::vector<WrapperLight> wrappers{};

    size_t GetByteSize() const { return ProtobufLight::Reflection::SerializedStructSize(*this); }
    bool ParseFromArray(const uint8_t* buffer, size_t size) { return ProtobufLight::Reflection::ParseStruct(*this, buffer, size); }
    std::string SerializeAsString() const { std::string out; ProtobufLight::Reflection::SerializeStruct(*this, out); return out; }
};

template<>
struct ProtobufLight::Reflection::ProtobufTrait<ItemLight>
{
    template<typename Obj, typename Callback>
    static void ForEachField(Obj& obj, Callback&& cb) {
        cb(obj.id, FieldMeta<1>{"id"});
        cb(obj.name, FieldMeta<2>{"name"});
    }
};

template<>
struct ProtobufLight::Reflection::ProtobufTrait<WrapperLight>
{
    template<typename Obj, typename Callback>
    static void ForEachField(Obj& obj, Callback&& cb) {
        cb(obj.single, FieldMeta<1>{"single"});
        cb(obj.many, FieldMeta<2>{"many"});
    }
};

template<>
struct ProtobufLight::Reflection::ProtobufTrait<RepeatedMessagesLight>
{
    template<typename Obj, typename Callback>
    static void ForEachField(Obj& obj, Callback&& cb) {
        cb(obj.items, FieldMeta<1>{"items"});
        cb(obj.wrappers, FieldMeta<2>{"wrappers"});
    }
};

