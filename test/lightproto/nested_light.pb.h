// Auto-generated from .proto
#pragma once

#include <ProtobufLight/ProtobufLightReflection.hpp>

struct LeafLight
{
    int64_t id{};

    size_t GetByteSize() const { return ProtobufLight::Reflection::SerializedStructSize(*this); }
    bool ParseFromArray(const uint8_t* buffer, size_t size) { return ProtobufLight::Reflection::ParseStruct(*this, buffer, size); }
    std::string SerializeAsString() const { std::string out; ProtobufLight::Reflection::SerializeStruct(*this, out); return out; }
};

struct MidLight
{
    LeafLight leaf{};
    std::vector<LeafLight> leaves{};

    size_t GetByteSize() const { return ProtobufLight::Reflection::SerializedStructSize(*this); }
    bool ParseFromArray(const uint8_t* buffer, size_t size) { return ProtobufLight::Reflection::ParseStruct(*this, buffer, size); }
    std::string SerializeAsString() const { std::string out; ProtobufLight::Reflection::SerializeStruct(*this, out); return out; }
};

struct OuterLight
{
    MidLight mid{};
    std::vector<MidLight> mids{};

    size_t GetByteSize() const { return ProtobufLight::Reflection::SerializedStructSize(*this); }
    bool ParseFromArray(const uint8_t* buffer, size_t size) { return ProtobufLight::Reflection::ParseStruct(*this, buffer, size); }
    std::string SerializeAsString() const { std::string out; ProtobufLight::Reflection::SerializeStruct(*this, out); return out; }
};

struct NestedAllLight
{
    OuterLight root{};
    std::vector<OuterLight> forest{};

    size_t GetByteSize() const { return ProtobufLight::Reflection::SerializedStructSize(*this); }
    bool ParseFromArray(const uint8_t* buffer, size_t size) { return ProtobufLight::Reflection::ParseStruct(*this, buffer, size); }
    std::string SerializeAsString() const { std::string out; ProtobufLight::Reflection::SerializeStruct(*this, out); return out; }
};

template<>
struct ProtobufLight::Reflection::ProtobufTrait<LeafLight>
{
    template<typename Obj, typename Callback>
    static void ForEachField(Obj& obj, Callback&& cb) {
        cb(obj.id, FieldMeta<1>{"id"});
    }
};

template<>
struct ProtobufLight::Reflection::ProtobufTrait<MidLight>
{
    template<typename Obj, typename Callback>
    static void ForEachField(Obj& obj, Callback&& cb) {
        cb(obj.leaf, FieldMeta<1>{"leaf"});
        cb(obj.leaves, FieldMeta<2>{"leaves"});
    }
};

template<>
struct ProtobufLight::Reflection::ProtobufTrait<OuterLight>
{
    template<typename Obj, typename Callback>
    static void ForEachField(Obj& obj, Callback&& cb) {
        cb(obj.mid, FieldMeta<1>{"mid"});
        cb(obj.mids, FieldMeta<2>{"mids"});
    }
};

template<>
struct ProtobufLight::Reflection::ProtobufTrait<NestedAllLight>
{
    template<typename Obj, typename Callback>
    static void ForEachField(Obj& obj, Callback&& cb) {
        cb(obj.root, FieldMeta<1>{"root"});
        cb(obj.forest, FieldMeta<2>{"forest"});
    }
};

