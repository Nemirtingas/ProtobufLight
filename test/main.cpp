
#include <iostream>

#include "protobuf.pb.h"
#include "protobuflight.pb.h"


int main(int argc, char* argv[])
{
	std::string protobuf_buffer, protobuf_light_buffer;

	ProtobufTest_pb test;
	ProtobufLightTest_pb light_test;

	protobuf_buffer = test.SerializeAsString();
	protobuf_light_buffer = light_test.SerializeAsString();

	if (protobuf_buffer != protobuf_light_buffer)
		std::cerr << "Invalid serialisation" << __LINE__ << std::endl;

	test.set_i32(5);
	light_test.i32 = 5;

	if (protobuf_buffer != protobuf_light_buffer)
		std::cerr << "Invalid serialisation" << __LINE__ << std::endl;

	test.mutable_inner()->set_a(99);
	light_test.inner.a = 99;

	if (protobuf_buffer != protobuf_light_buffer)
		std::cerr << "Invalid serialisation" << __LINE__ << std::endl;

	test.mutable_oneofinner4()->set_a(12);
	light_test.oneofInner = 12;

	if (protobuf_buffer != protobuf_light_buffer)
		std::cerr << "Invalid serialisation" << __LINE__ << std::endl;

	test.set_d(3.0);
	light_test.d = 3.0;

	if (protobuf_buffer != protobuf_light_buffer)
		std::cerr << "Invalid serialisation" << __LINE__ << std::endl;

	test.set_f(2.0f);
	light_test.f = 2.0f;

	if (protobuf_buffer != protobuf_light_buffer)
		std::cerr << "Invalid serialisation" << __LINE__ << std::endl;

	test.set_s("test");
	light_test.s = "test";

	if (protobuf_buffer != protobuf_light_buffer)
		std::cerr << "Invalid serialisation" << __LINE__ << std::endl;

	test.add_repeated()->set_a(1);
	light_test.repeated.emplace_back().a = 1;

	if (protobuf_buffer != protobuf_light_buffer)
		std::cerr << "Invalid serialisation" << __LINE__ << std::endl;

	test.add_repeated()->set_a(2);
	light_test.repeated.emplace_back().a = 2;
	test.add_repeated()->set_a(3);
	light_test.repeated.emplace_back().a = 3;

	if (protobuf_buffer != protobuf_light_buffer)
		std::cerr << "Invalid serialisation" << __LINE__ << std::endl;

	test.add_repeated_int(1);
	light_test.repeated_int.emplace_back(1);
	test.add_repeated_int(2);
	light_test.repeated_int.emplace_back(2);
	test.add_repeated_int(3);
	light_test.repeated_int.emplace_back(3);

	if (protobuf_buffer != protobuf_light_buffer)
		std::cerr << "Invalid serialisation" << __LINE__ << std::endl;

	(*test.mutable_optional()) = "optional";
	light_test.optional = "optional";

	return 0;
}