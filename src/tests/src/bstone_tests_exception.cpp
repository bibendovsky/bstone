#include <stdexcept>
#include <string>

#include "bstone_tester.h"

#include "bstone_exception.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// StaticSourceException(const SourceLocation&, const char*)
void test_9dy6ii4t7of8saop()
{
	const auto reference_file_name = std::string{"file_name"};
	const auto reference_line = 1'234'567'890;
	const auto reference_function_name = std::string{"function_name"};
	const auto reference_message = std::string{"message"};

	const auto reference_source_location = bstone::SourceLocation{
		reference_file_name.c_str(),
		reference_line,
		reference_function_name.c_str()};

	const auto exception = bstone::StaticSourceException{
		reference_source_location,
		reference_message.c_str()};

	const auto& source_location = exception.get_source_location();
	const auto file_name = source_location.get_file_name();
	const auto line = source_location.get_line();
	const auto function_name = source_location.get_function_name();
	const auto message = exception.what();

	tester.check(
		file_name == reference_file_name.c_str() &&
		std::string{file_name} == reference_file_name &&
		line == reference_line &&
		function_name == reference_function_name.c_str() &&
		std::string{function_name} == reference_function_name &&
		message == reference_message.c_str() &&
		std::string{message} == reference_message);
}

// StaticSourceException(const SourceLocation&)
void test_8ojkzshd2x7805in()
{
	const auto reference_file_name = std::string{"file_name"};
	const auto reference_line = 1'234'567'890;
	const auto reference_function_name = std::string{"function_name"};
	const auto reference_message = std::string{};

	const auto reference_source_location = bstone::SourceLocation{
		reference_file_name.c_str(),
		reference_line,
		reference_function_name.c_str()};

	const auto exception = bstone::StaticSourceException{reference_source_location};

	const auto& source_location = exception.get_source_location();
	const auto file_name = source_location.get_file_name();
	const auto line = source_location.get_line();
	const auto function_name = source_location.get_function_name();
	const auto message = exception.what();

	tester.check(
		file_name == reference_file_name.c_str() &&
		std::string{file_name} == reference_file_name &&
		line == reference_line &&
		function_name == reference_function_name.c_str() &&
		std::string{function_name} == reference_function_name &&
		message != nullptr &&
		std::string{message} == reference_message);
}

// StaticSourceException(const StaticSourceException&)
void test_9v8dcb77m5xpydb5()
{
	const auto reference_file_name = std::string{"file_name"};
	const auto reference_line = 1'234'567'890;
	const auto reference_function_name = std::string{"function_name"};
	const auto reference_message = std::string{"message"};

	const auto reference_source_location = bstone::SourceLocation{
		reference_file_name.c_str(),
		reference_line,
		reference_function_name.c_str()};

	const auto exception_1 = bstone::StaticSourceException{
		reference_source_location,
		reference_message.c_str()};

	const auto exception = exception_1;

	const auto& source_location = exception.get_source_location();
	const auto file_name = source_location.get_file_name();
	const auto line = source_location.get_line();
	const auto function_name = source_location.get_function_name();
	const auto message = exception.what();

	tester.check(
		file_name == reference_file_name.c_str() &&
		std::string{file_name} == reference_file_name &&
		line == reference_line &&
		function_name == reference_function_name.c_str() &&
		std::string{function_name} == reference_function_name &&
		message == reference_message.c_str() &&
		std::string{message} == reference_message);
}

// operator=(const StaticSourceException&)
void test_vnunxr60u5dxm6b2()
{
	const auto reference_file_name = std::string{"file_name"};
	const auto reference_line = 1'234'567'890;
	const auto reference_function_name = std::string{"function_name"};
	const auto reference_message = std::string{"message"};

	const auto reference_source_location = bstone::SourceLocation{
		reference_file_name.c_str(),
		reference_line,
		reference_function_name.c_str()};

	const auto exception_1 = bstone::StaticSourceException{
		reference_source_location,
		reference_message.c_str()};

	auto exception = bstone::StaticSourceException{
		bstone::SourceLocation{"???", 1, "???"},
		"???"};

	exception = exception_1;

	const auto& source_location = exception.get_source_location();
	const auto file_name = source_location.get_file_name();
	const auto line = source_location.get_line();
	const auto function_name = source_location.get_function_name();
	const auto message = exception.what();

	tester.check(
		file_name == reference_file_name.c_str() &&
		std::string{file_name} == reference_file_name &&
		line == reference_line &&
		function_name == reference_function_name.c_str() &&
		std::string{function_name} == reference_function_name &&
		message == reference_message.c_str() &&
		std::string{message} == reference_message);
}

// get_source_location()
void test_th2929otnou2oism()
{
	const auto reference_file_name = std::string{"file_name"};
	const auto reference_line = 1'234'567'890;
	const auto reference_function_name = std::string{"function_name"};
	const auto reference_message = std::string{"message"};

	const auto reference_source_location = bstone::SourceLocation{
		reference_file_name.c_str(),
		reference_line,
		reference_function_name.c_str()};

	const auto exception = bstone::StaticSourceException{
		reference_source_location,
		reference_message.c_str()};

	const auto& source_location = exception.get_source_location();
	const auto file_name = source_location.get_file_name();
	const auto line = source_location.get_line();
	const auto function_name = source_location.get_function_name();

	tester.check(
		file_name == reference_file_name.c_str() &&
		std::string{file_name} == reference_file_name &&
		line == reference_line &&
		function_name == reference_function_name.c_str() &&
		std::string{function_name} == reference_function_name);
}

// what()
void test_g2d1isjrlhiwx9n4()
{
	const auto reference_message = std::string{"message"};
	const auto reference_source_location = bstone::SourceLocation{"???", 1, "???"};

	const auto exception = bstone::StaticSourceException{
		reference_source_location,
		reference_message.c_str()};

	const auto message = exception.what();
	tester.check(message == reference_message.c_str() && std::string{message} == reference_message);
}

// fail(const SourceLocation&, const char*)
void test_kh63aix1vb0xiyhv()
{
	const auto reference_file_name = std::string{"file_name"};
	const auto reference_line = 1'234'567'890;
	const auto reference_function_name = std::string{"function_name"};
	const auto reference_message = std::string{"message"};

	const auto reference_source_location = bstone::SourceLocation{
		reference_file_name.c_str(),
		reference_line,
		reference_function_name.c_str()};

	try
	{
		bstone::StaticSourceException::fail(reference_source_location, reference_message.c_str());
	}
	catch (const bstone::StaticSourceException& exception)
	{
		const auto& source_location = exception.get_source_location();
		const auto file_name = source_location.get_file_name();
		const auto line = source_location.get_line();
		const auto function_name = source_location.get_function_name();
		const auto message = exception.what();

		tester.check(
			file_name == reference_file_name.c_str() &&
			std::string{file_name} == reference_file_name &&
			line == reference_line &&
			function_name == reference_function_name.c_str() &&
			std::string{function_name} == reference_function_name &&
			message == reference_message.c_str() &&
			std::string{message} == reference_message);
	}
}

// fail_nested(const SourceLocation&)
void test_zcs3c4u46sekri7a()
{
	const auto reference_inner_message = std::string{"inner"};

	const auto reference_file_name = std::string{"file_name"};
	const auto reference_line = 1'234'567'890;
	const auto reference_function_name = std::string{"function_name"};
	const auto reference_message = std::string{};

	const auto reference_source_location = bstone::SourceLocation{
		reference_file_name.c_str(),
		reference_line,
		reference_function_name.c_str()};

	try
	{
		try
		{
			throw std::runtime_error{reference_inner_message};
		}
		catch (const std::exception&)
		{
			bstone::StaticSourceException::fail_nested(reference_source_location);
		}
	}
	catch (const bstone::StaticSourceException& exception)
	{
		const auto& source_location = exception.get_source_location();
		const auto file_name = source_location.get_file_name();
		const auto line = source_location.get_line();
		const auto function_name = source_location.get_function_name();
		const auto message = exception.what();

		tester.check(
			file_name == reference_file_name.c_str() &&
			std::string{file_name} == reference_file_name &&
			line == reference_line &&
			function_name == reference_function_name.c_str() &&
			std::string{function_name} == reference_function_name &&
			message != nullptr &&
			std::string{message} == reference_message);

		auto is_runtime_error = false;

		try
		{
			std::rethrow_if_nested(exception);
		}
		catch (const std::runtime_error& runtime_exception)
		{
			const auto inner_message = runtime_exception.what();

			is_runtime_error =
				inner_message != nullptr &&
				std::string{inner_message} == reference_inner_message;
		}

		tester.check(is_runtime_error, "e8xcbfhc9xl2invy");
	}
}

// ==========================================================================

// DynamicSourceException(const SourceLocation&, const char*)
void test_mwd0ds2bxczncdyi()
{
	const auto reference_file_name = std::string{"file_name"};
	const auto reference_line = 1'234'567'890;
	const auto reference_function_name = std::string{"function_name"};
	const auto reference_message = std::string{"message"};

	const auto reference_source_location = bstone::SourceLocation{
		reference_file_name.c_str(),
		reference_line,
		reference_function_name.c_str()};

	const auto exception = bstone::DynamicSourceException{reference_source_location, reference_message.c_str()};
	const auto& source_location = exception.get_source_location();
	const auto file_name = source_location.get_file_name();
	const auto line = source_location.get_line();
	const auto function_name = source_location.get_function_name();
	const auto message = exception.what();

	tester.check(
		file_name == reference_file_name.c_str() &&
		std::string{file_name} == reference_file_name &&
		line == reference_line &&
		function_name == reference_function_name.c_str() &&
		std::string{function_name} == reference_function_name &&
		message != nullptr &&
		std::string{message} == reference_message);
}

// DynamicSourceException(const DynamicSourceException&)
void test_0r0rt838e8vmvyv3()
{
	const auto reference_file_name = std::string{"file_name"};
	const auto reference_line = 1'234'567'890;
	const auto reference_function_name = std::string{"function_name"};
	const auto reference_message = std::string{"message"};

	const auto reference_source_location = bstone::SourceLocation{
		reference_file_name.c_str(),
		reference_line,
		reference_function_name.c_str()};

	const auto exception_1 = bstone::DynamicSourceException{reference_source_location, reference_message.c_str()};
	const auto exception = exception_1;

	const auto& source_location = exception.get_source_location();
	const auto file_name = source_location.get_file_name();
	const auto line = source_location.get_line();
	const auto function_name = source_location.get_function_name();

	const auto message_1 = exception_1.what();
	const auto message = exception.what();

	tester.check(
		file_name == reference_file_name.c_str() &&
		std::string{file_name} == reference_file_name &&
		line == reference_line &&
		function_name == reference_function_name.c_str() &&
		std::string{function_name} == reference_function_name &&
		message_1 != nullptr &&
		message != nullptr &&
		message == message_1 &&
		std::string{message} == reference_message);
}

// operator=(const DynamicSourceException&)
void test_1jezs70lk1p3q1fm()
{
	const auto reference_file_name = std::string{"file_name"};
	const auto reference_line = 1'234'567'890;
	const auto reference_function_name = std::string{"function_name"};
	const auto reference_message = std::string{"message"};

	const auto reference_source_location = bstone::SourceLocation{
		reference_file_name.c_str(),
		reference_line,
		reference_function_name.c_str()};

	const auto exception_1 = bstone::DynamicSourceException{reference_source_location, reference_message.c_str()};

	auto exception = bstone::DynamicSourceException{
		bstone::SourceLocation{"???", 0, "???"},
		"???"};

	exception = exception_1;

	const auto& source_location = exception.get_source_location();
	const auto file_name = source_location.get_file_name();
	const auto line = source_location.get_line();
	const auto function_name = source_location.get_function_name();

	const auto message_1 = exception_1.what();
	const auto message = exception.what();

	tester.check(
		file_name == reference_file_name.c_str() &&
		std::string{file_name} == reference_file_name &&
		line == reference_line &&
		function_name == reference_function_name.c_str() &&
		std::string{function_name} == reference_function_name &&
		message_1 != nullptr &&
		message != nullptr &&
		message == message_1 &&
		std::string{message} == reference_message);
}

// get_source_location()
void test_xm6p0s1xcnghpeam()
{
	const auto reference_file_name = std::string{"file_name"};
	const auto reference_line = 1'234'567'890;
	const auto reference_function_name = std::string{"function_name"};
	const auto reference_message = std::string{"message"};

	const auto reference_source_location = bstone::SourceLocation{
		reference_file_name.c_str(),
		reference_line,
		reference_function_name.c_str()};

	const auto exception = bstone::DynamicSourceException{reference_source_location, reference_message.c_str()};

	const auto& source_location = exception.get_source_location();
	const auto file_name = source_location.get_file_name();
	const auto line = source_location.get_line();
	const auto function_name = source_location.get_function_name();

	tester.check(
		file_name == reference_file_name.c_str() &&
		std::string{file_name} == reference_file_name &&
		line == reference_line &&
		function_name == reference_function_name.c_str() &&
		std::string{function_name} == reference_function_name);
}

// what()
void test_c2hepxe4z811dptg()
{
	const auto reference_file_name = std::string{"file_name"};
	const auto reference_line = 1'234'567'890;
	const auto reference_function_name = std::string{"function_name"};
	const auto reference_message = std::string{"message"};

	const auto reference_source_location = bstone::SourceLocation{
		reference_file_name.c_str(),
		reference_line,
		reference_function_name.c_str()};

	const auto exception = bstone::DynamicSourceException{reference_source_location, reference_message.c_str()};
	const auto message = exception.what();

	tester.check(message != nullptr && std::string{message} == reference_message);
}

// swap(DynamicSourceException&)
void test_f2xqhq2x2zv1yndi()
{
	const auto reference_file_name = std::string{"file_name"};
	const auto reference_line = 1'234'567'890;
	const auto reference_function_name = std::string{"function_name"};
	const auto reference_message = std::string{"message"};

	const auto reference_source_location = bstone::SourceLocation{
		reference_file_name.c_str(),
		reference_line,
		reference_function_name.c_str()};

	auto exception_1 = bstone::DynamicSourceException{reference_source_location, reference_message.c_str()};

	auto exception = bstone::DynamicSourceException{
		bstone::SourceLocation{"???", 0, "???"},
		"???"};

	exception.swap(exception_1);

	const auto& source_location = exception.get_source_location();
	const auto file_name = source_location.get_file_name();
	const auto line = source_location.get_line();
	const auto function_name = source_location.get_function_name();
	const auto message = exception.what();

	tester.check(
		file_name == reference_file_name.c_str() &&
		std::string{file_name} == reference_file_name &&
		line == reference_line &&
		function_name == reference_function_name.c_str() &&
		std::string{function_name} == reference_function_name &&
		message != nullptr &&
		std::string{message} == reference_message);
}

// fail(const SourceLocation&, const char*)
void test_hm79m48ddfmlphzq()
{
	const auto reference_file_name = std::string{"file_name"};
	const auto reference_line = 1'234'567'890;
	const auto reference_function_name = std::string{"function_name"};
	const auto reference_message = std::string{"message"};

	const auto reference_source_location = bstone::SourceLocation{
		reference_file_name.c_str(),
		reference_line,
		reference_function_name.c_str()};

	try
	{
		throw bstone::StaticSourceException{reference_source_location, reference_message.c_str()};
	}
	catch (const bstone::StaticSourceException& exception)
	{
		const auto& source_location = exception.get_source_location();
		const auto file_name = source_location.get_file_name();
		const auto line = source_location.get_line();
		const auto function_name = source_location.get_function_name();
		const auto message = exception.what();

		tester.check(
			file_name == reference_file_name.c_str() &&
			std::string{file_name} == reference_file_name &&
			line == reference_line &&
			function_name == reference_function_name.c_str() &&
			std::string{function_name} == reference_function_name &&
			message == reference_message.c_str() &&
			std::string{message} == reference_message);
	}
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_static_source_exception();
		register_dynamic_source_exception();
	}

private:
	void register_static_source_exception()
	{
		tester.register_test("StaticSourceException#9dy6ii4t7of8saop", test_9dy6ii4t7of8saop);
		tester.register_test("StaticSourceException#8ojkzshd2x7805in", test_8ojkzshd2x7805in);
		tester.register_test("StaticSourceException#9v8dcb77m5xpydb5", test_9v8dcb77m5xpydb5);
		tester.register_test("StaticSourceException#vnunxr60u5dxm6b2", test_vnunxr60u5dxm6b2);
		tester.register_test("StaticSourceException#th2929otnou2oism", test_th2929otnou2oism);
		tester.register_test("StaticSourceException#g2d1isjrlhiwx9n4", test_g2d1isjrlhiwx9n4);
		tester.register_test("StaticSourceException#kh63aix1vb0xiyhv", test_kh63aix1vb0xiyhv);
		tester.register_test("StaticSourceException#zcs3c4u46sekri7a", test_zcs3c4u46sekri7a);
	}

	void register_dynamic_source_exception()
	{
		tester.register_test("DynamicSourceException#mwd0ds2bxczncdyi", test_mwd0ds2bxczncdyi);
		tester.register_test("DynamicSourceException#0r0rt838e8vmvyv3", test_0r0rt838e8vmvyv3);
		tester.register_test("DynamicSourceException#1jezs70lk1p3q1fm", test_1jezs70lk1p3q1fm);
		tester.register_test("DynamicSourceException#xm6p0s1xcnghpeam", test_xm6p0s1xcnghpeam);
		tester.register_test("DynamicSourceException#c2hepxe4z811dptg", test_c2hepxe4z811dptg);
		tester.register_test("DynamicSourceException#f2xqhq2x2zv1yndi", test_f2xqhq2x2zv1yndi);
		tester.register_test("DynamicSourceException#hm79m48ddfmlphzq", test_hm79m48ddfmlphzq);
	}
};

auto registrator = Registrator{};

} // namespace
