#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include <string>
#include <iostream>

#include "proto/scalars.pb.h"
#include "proto/repeated_scalars.pb.h"
#include "proto/repeated_messages.pb.h"
#include "proto/maps_scalars.pb.h"
#include "proto/maps_messages.pb.h"
#include "proto/oneof_all.pb.h"
#include "proto/nested.pb.h"
#include "proto/optional_presence.pb.h"
#include "proto/defaults.pb.h"
#include "proto/compat_v1.pb.h"
#include "proto/compat_v2.pb.h"

#include "lightproto/scalars_light.pb.h"
#include "lightproto/repeated_scalars_light.pb.h"
#include "lightproto/repeated_messages_light.pb.h"
#include "lightproto/maps_scalars_light.pb.h"
#include "lightproto/maps_messages_light.pb.h"
#include "lightproto/oneof_all_light.pb.h"
#include "lightproto/nested_light.pb.h"
#include "lightproto/optional_presence_light.pb.h"
#include "lightproto/defaults_light.pb.h"
#include "lightproto/compat_v1_light.pb.h"
#include "lightproto/compat_v2_light.pb.h"

using namespace std;

int main(int argc, char* argv[])
{
    return Catch::Session().run(argc, argv);
}

static bool compareBuffers(const std::string& a, const std::string& b) {
    if (a == b) return true;
    size_t n = std::min(a.size(), b.size());
    for (size_t i = 0; i < n; i++) {
        if (a[i] != b[i]) {
            std::cerr << "Buffers differ at byte " << i
                << " A=" << (int)(unsigned char)a[i]
                << " B=" << (int)(unsigned char)b[i] << "\n";
                break;
        }
    }
    return false;
}

template<typename A, typename B>
static void roundtrip(const A& a, const B& b) {
    std::string sa, sb;
    REQUIRE(a.SerializeToString(&sa));
    sb = b.SerializeAsString();
    REQUIRE(compareBuffers(sa, sb));

    A a2; B b2;
    REQUIRE(a2.ParseFromString(sb));
    REQUIRE(b2.ParseFromArray(reinterpret_cast<const uint8_t*>(sa.data()), sa.size()));

    std::string sa2, sb2;
    REQUIRE(a2.SerializeToString(&sa2));
    sb2 = b2.SerializeAsString();
    REQUIRE(compareBuffers(sa2, sb2));
}

// ---------------------- Tests ----------------------

TEST_CASE("Scalars") {
    Scalars g;
    g.set_f_int32(42);
    g.set_f_string("hello");
    g.set_f_enum(TestEnum::ENUM_TWO);

    ScalarsLight l;
    l.f_int32 = 42;
    l.f_string = "hello";
    l.f_enum = TestEnumLight::ENUM_TWO;

    roundtrip(g, l);
}

TEST_CASE("Repeated scalars") {
    RepeatedScalars g;
    g.add_r_int32_default_packed(1);
    g.add_r_int32_default_packed(2);
    g.add_r_strings("a");
    g.add_r_strings("a");

    RepeatedScalarsLight l;
    l.r_int32_default_packed.emplace_back(1);
    l.r_int32_default_packed.emplace_back(2);
    l.r_strings.emplace_back("a");
    l.r_strings.emplace_back("a");

    roundtrip(g, l);
}

TEST_CASE("Repeated messages") {
    Item* i = nullptr;
    RepeatedMessages g;
    i = g.add_items();
    i->set_id(1);
    i->set_name("x");

    RepeatedMessagesLight l;
    auto& il = l.items.emplace_back();
    il.id = 1;
    il.name = "x";

    roundtrip(g, l);
}

TEST_CASE("Maps scalars") {
    MapsScalars g;
    (*g.mutable_m_str_i32())["a"] = 1;
    MapsScalarsLight l;
    l.m_str_i32["a"] = 1;

    roundtrip(g, l);
}

TEST_CASE("Maps messages") {
    MapsMessages g;
    (*g.mutable_m_str_msg())["k"].set_a(7);
    MapsMessagesLight l;
    l.m_str_msg["k"].a = 7;

    roundtrip(g, l);
}

TEST_CASE("Oneof") {
    OneOfAll g;
    g.set_o_int32(99);

    OneOfAllLight l;
    l.choice = 99;

    roundtrip(g, l);
}

TEST_CASE("Nested") {
    NestedAll g;
    auto* mid = g.mutable_root()->mutable_mid();
    mid->mutable_leaf()->set_id(123);

    NestedAllLight l;
    auto& midl = l.root.mid;
    midl.leaf.id = 123;

    roundtrip(g, l);
}

TEST_CASE("Optional") {
    OptionalPresence g;
    g.set_o_int32(10);

    OptionalPresenceLight l;
    l.o_int32 = 10;

    roundtrip(g, l);
}

TEST_CASE("Defaults") {
    Defaults g;
    DefaultsLight l;

    roundtrip(g, l);
}

TEST_CASE("Compat V1 vs V2") {
    CompatV1 g;
    g.set_a(1);
    g.set_b("x");

    CompatV1Light l;
    l.a = 1;
    l.b = "x";

    roundtrip(g, l);

    CompatV2 g2;
    g2.set_a(1);
    g2.set_b("x");
    g2.set_c(999);
    g2.set_d("raw");

    CompatV2Light l2;
    l2.a = 1;
    l2.b = "x";
    l2.c = 999;
    l2.d = "raw";

    roundtrip(g2, l2);
}
