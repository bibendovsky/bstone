#include "bstone_tester.h"

#include "bstone_memory.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

void test_3mpewjrxifo6nul9()
{
	constexpr int reference_value = -1'234'567'890;
	int storage = 0;
	const auto storage_ptr = bstone::construct_at(&storage, reference_value);
	tester.check(storage_ptr == &storage && storage == reference_value);
}

void test_3ywzqi8lwhgv44ik()
{
	constexpr int reference_value = -1'234'567'890;

	int tag = 0;

	class Storage
	{
	public:
		Storage(int& tag)
			:
			tag_{tag}
		{
		}

		Storage(const Storage&) = delete;
		Storage& operator=(const Storage&) = delete;

		~Storage()
		{
			tag_ = reference_value;
		}

	private:
		int& tag_;
	};

	Storage storage{tag};
	bstone::destroy_at(&storage);
	tester.check(tag == reference_value);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_construct_at();
		register_destroy_at();
	}

private:
	void register_construct_at()
	{
		tester.register_test("construct_at#3mpewjrxifo6nul9", test_3mpewjrxifo6nul9);
	}

	void register_destroy_at()
	{
		tester.register_test("destroy_at#3ywzqi8lwhgv44ik", test_3ywzqi8lwhgv44ik);
	}
};

auto registrator = Registrator{};

} // namespace
