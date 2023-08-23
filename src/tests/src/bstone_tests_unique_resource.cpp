#include <utility>

#include "bstone_tester.h"

#include "bstone_unique_resource.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================
// Monostate.

// get_empty_value()
void test_5vsgrh0ngiztibku()
{
	const auto ref_empty_value = -1;

	struct EmptyValue
	{
		int operator()() const noexcept
		{
			return ref_empty_value;
		}
	};

	struct Deleter
	{
		void operator()(int) const noexcept {}
	};

	using UR = bstone::UniqueResource<int, Deleter, EmptyValue>;
	const auto empty_value = UR::get_empty_value();
	tester.check(empty_value == ref_empty_value, "5vsgrh0ngiztibku");
}

// UniqueResource()
void test_nwhuwglw2a6jo9d6()
{
	const auto ref_empty_value = -1;

	struct EmptyValue
	{
		int operator()() const noexcept
		{
			return ref_empty_value;
		}
	};

	struct Deleter
	{
		void operator()(int) const noexcept {}
	};

	using UR = bstone::UniqueResource<int, Deleter, EmptyValue>;
	const auto ur = UR{};
	tester.check(ur.get_empty_value() == ref_empty_value, "nwhuwglw2a6jo9d6");
}

// UniqueResource(Resource)
void test_p21v8j553fgt2lal()
{
	const auto ref_value = 100;
	const auto ref_empty_value = -1;

	struct EmptyValue
	{
		int operator()() const noexcept
		{
			return ref_empty_value;
		}
	};

	struct Deleter
	{
		void operator()(int) const noexcept {}
	};

	using UR = bstone::UniqueResource<int, Deleter, EmptyValue>;
	const auto ur = UR{ref_value};
	tester.check(ur.get() == ref_value, "p21v8j553fgt2lal");
}

// UniqueResource(UniqueResource&&)
auto counter_tf5z5j5up4apbh0b = 0;

void test_tf5z5j5up4apbh0b()
{
	const auto ref_value = 100;
	const auto ref_empty_value = -1;

	struct EmptyValue
	{
		int operator()() const noexcept
		{
			return ref_empty_value;
		}
	};

	struct Deleter
	{
		void operator()(int) const noexcept
		{
			++counter_tf5z5j5up4apbh0b;
		}
	};

	auto is_equals = false;

	{
		using UR = bstone::UniqueResource<int, Deleter, EmptyValue>;
		auto ur = UR{UR{ref_value}};
		is_equals = ur.get() == ref_value;
	}

	tester.check(is_equals && counter_tf5z5j5up4apbh0b == 1, "tf5z5j5up4apbh0b");
}

// UniqueResource& operator=(UniqueResource&&)
auto counter_0q6l99sxup31qxq9 = 0;

void test_0q6l99sxup31qxq9()
{
	const auto ref_value = 100;
	const auto ref_empty_value = -1;

	struct EmptyValue
	{
		int operator()() const noexcept
		{
			return ref_empty_value;
		}
	};

	struct Deleter
	{
		void operator()(int) const noexcept
		{
			++counter_0q6l99sxup31qxq9;
		}
	};

	auto is_equal = false;

	{
		using UR = bstone::UniqueResource<int, Deleter, EmptyValue>;
		auto ur1 = UR{ref_value};
		auto ur = UR{};
		ur = std::move(ur1);
		is_equal = ur.get() == ref_value;
	}

	tester.check(is_equal && counter_0q6l99sxup31qxq9 == 1, "0q6l99sxup31qxq9");
}

// get()
void test_nuixo0tzra25rt5f()
{
	const auto ref_empty_value = -1;

	struct EmptyValue
	{
		int operator()() const noexcept
		{
			return ref_empty_value;
		}
	};

	struct Deleter
	{
		void operator()(int) const noexcept {}
	};

	using UR = bstone::UniqueResource<int, Deleter, EmptyValue>;
	const auto ur = UR{};
	tester.check(ur.get() == ref_empty_value, "nuixo0tzra25rt5f");
}

// is_empty()
void test_5yyqy7ylaj5upttc()
{
	const auto ref_empty_value = -1;

	struct EmptyValue
	{
		int operator()() const noexcept
		{
			return ref_empty_value;
		}
	};

	struct Deleter
	{
		void operator()(int) const noexcept {}
	};

	using UR = bstone::UniqueResource<int, Deleter, EmptyValue>;
	const auto ur = UR{};
	tester.check(ur.is_empty(), "5yyqy7ylaj5upttc");
}

auto counter_i0gki9p89yl7xv1i = 0;

// reset()
void test_i0gki9p89yl7xv1i()
{
	const auto ref_empty_value = -1;

	struct EmptyValue
	{
		int operator()() const noexcept
		{
			return ref_empty_value;
		}
	};

	struct Deleter
	{
		void operator()(int) const noexcept
		{
			++counter_i0gki9p89yl7xv1i;
		}
	};

	auto is_equal = false;

	{
		using UR = bstone::UniqueResource<int, Deleter, EmptyValue>;
		auto ur = UR{100};
		ur.reset();
		is_equal = ur.get() == ref_empty_value;
	}

	tester.check(is_equal, "i0gki9p89yl7xv1i");
}

auto counter_aa2twi6yj4dm1mvo = 0;

// reset(Resource)
void test_aa2twi6yj4dm1mvo()
{
	const auto ref_initial_value = 100;
	const auto ref_value = 1000;
	const auto ref_empty_value = -1;

	struct EmptyValue
	{
		int operator()() const noexcept
		{
			return ref_empty_value;
		}
	};

	struct Deleter
	{
		void operator()(int) const noexcept
		{
			++counter_aa2twi6yj4dm1mvo;
		}
	};

	auto is_equal = false;

	{
		using UR = bstone::UniqueResource<int, Deleter, EmptyValue>;
		auto ur = UR{ref_initial_value};
		ur.reset(ref_value);
		is_equal = ur.get() == ref_value;
	}

	tester.check(is_equal && counter_aa2twi6yj4dm1mvo == 2, "aa2twi6yj4dm1mvo");
}

// release()
auto counter_qezkdligeaj4qjaq = 0;

void test_qezkdligeaj4qjaq()
{
	const auto ref_value = 100;
	const auto ref_empty_value = -1;

	struct EmptyValue
	{
		int operator()() const noexcept
		{
			return ref_empty_value;
		}
	};

	struct Deleter
	{
		void operator()(int) const noexcept
		{
			++counter_qezkdligeaj4qjaq;
		}
	};

	auto is_equal = false;

	{
		using UR = bstone::UniqueResource<int, Deleter, EmptyValue>;
		auto ur = UR{ref_value};
		const auto value = ur.release();
		is_equal = ur.get() == ref_empty_value && value == ref_value;
	}

	tester.check(is_equal && counter_qezkdligeaj4qjaq == 0, "qezkdligeaj4qjaq");
}

// swap(UniqueResource&)
void test_ri9ibhitw8i5tnle()
{
	const auto ref_value = 1000;
	const auto ref_empty_value = -1;

	struct EmptyValue
	{
		int operator()() const noexcept
		{
			return ref_empty_value;
		}
	};

	struct Deleter
	{
		void operator()(int) const noexcept {}
	};

	using UR = bstone::UniqueResource<int, Deleter, EmptyValue>;
	auto ur1 = UR{ref_value};
	auto ur = UR{};
	ur1.swap(ur);

	tester.check(ur1.get() == ref_empty_value && ur.get() == ref_value, "ri9ibhitw8i5tnle");
}

// operator*()
void test_58aiayzra6imm0p8()
{
	const auto ref_value = 1000;

	struct Deleter
	{
		void operator()(int*) const noexcept {}
	};

	using UR = bstone::UniqueResource<int*, Deleter>;
	auto value = ref_value;
	const auto ur = UR{&value};
	const auto result = *ur;

	tester.check(result == ref_value && ur.get() == &value, "58aiayzra6imm0p8");
}

// operator->()
void test_fp3toubop8s4t805()
{
	const auto ref_value = 1000;

	struct Value
	{
		int x;
	};

	struct Deleter
	{
		void operator()(Value*) const noexcept {}
	};

	using UR = bstone::UniqueResource<Value*, Deleter>;
	auto value = Value{ref_value};
	auto ur = UR{&value};
	const auto result = ur->x;

	tester.check(result == ref_value && ur.get() == &value, "fp3toubop8s4t805");
}

// bool operator==(const UniqueResource&, const UniqueResource&)
// Equal.
void test_iow0ep4j9jhr0s40()
{
	const auto ref_value = 100;

	struct Deleter
	{
		void operator()(int) const noexcept {}
	};

	using UR = bstone::UniqueResource<int, Deleter>;
	const auto ur1 = UR{ref_value};
	const auto ur2 = UR{ref_value};
	tester.check(ur1 == ur2, "iow0ep4j9jhr0s40");
}

// bool operator==(const UniqueResource&, const UniqueResource&)
// Not equal.
void test_895zkfoibw1c5eu0()
{
	const auto ref_value1 = 100;
	const auto ref_value2 = 200;

	struct Deleter
	{
		void operator()(int) const noexcept {}
	};

	using UR = bstone::UniqueResource<int, Deleter>;
	const auto ur1 = UR{ref_value1};
	const auto ur2 = UR{ref_value2};
	tester.check(!(ur1 == ur2), "895zkfoibw1c5eu0");
}

// bool operator!=(const UniqueResource&, const UniqueResource&)
// Not equal.
void test_884glztjux6fffet()
{
	const auto ref_value1 = 100;
	const auto ref_value2 = 200;

	struct Deleter
	{
		void operator()(int) const noexcept {}
	};

	using UR = bstone::UniqueResource<int, Deleter>;
	const auto ur1 = UR{ref_value1};
	const auto ur2 = UR{ref_value2};
	tester.check(ur1 != ur2, "884glztjux6fffet");
}

// bool operator!=(const UniqueResource&, const UniqueResource&)
// Equal.
void test_ck0h8bfcyrwo4amu()
{
	const auto ref_value = 100;

	struct Deleter
	{
		void operator()(int) const noexcept {}
	};

	using UR = bstone::UniqueResource<int, Deleter>;
	const auto ur1 = UR{ref_value};
	const auto ur2 = UR{ref_value};
	tester.check(!(ur1 != ur2), "ck0h8bfcyrwo4amu");
}

// bool operator==(const UniqueResource&, nullptr)
// Equal.
void test_tcppddfvbsfxx8g7()
{
	struct Deleter
	{
		void operator()(int*) const noexcept {}
	};

	using UR = bstone::UniqueResource<int*, Deleter>;
	const auto ur = UR{};
	tester.check(ur == nullptr, "tcppddfvbsfxx8g7");
}

// bool operator==(const UniqueResource&, nullptr)
// Not equal.
void test_rr3h438f26wnkhgy()
{
	struct Deleter
	{
		void operator()(int*) const noexcept {}
	};

	using UR = bstone::UniqueResource<int*, Deleter>;
	auto value = 100;
	const auto ur = UR{&value};
	tester.check(!(ur == nullptr), "rr3h438f26wnkhgy");
}

// bool operator!=(const UniqueResource&, nullptr)
// Not equal.
void test_rln6v214wkr5bij1()
{
	struct Deleter
	{
		void operator()(int*) const noexcept {}
	};

	using UR = bstone::UniqueResource<int*, Deleter>;
	auto value = 100;
	const auto ur = UR{&value};
	tester.check(ur != nullptr, "rln6v214wkr5bij1");
}

// bool operator!=(const UniqueResource&, nullptr)
// Not equal.
void test_q6biwiux463ru724()
{
	struct Deleter
	{
		void operator()(int*) const noexcept {}
	};

	using UR = bstone::UniqueResource<int*, Deleter>;
	const auto ur = UR{};
	tester.check(!(ur != nullptr), "q6biwiux463ru724");
}

// bool operator==(nullptr, const UniqueResource&)
// Equal.
void test_0z0gmpop34d4d8ma()
{
	struct Deleter
	{
		void operator()(int*) const noexcept {}
	};

	using UR = bstone::UniqueResource<int*, Deleter>;
	const auto ur = UR{};
	tester.check(nullptr == ur, "0z0gmpop34d4d8ma");
}

// bool operator==(nullptr, const UniqueResource&)
// Not equal.
void test_eoae7a2oaxd7c1zx()
{
	struct Deleter
	{
		void operator()(int*) const noexcept {}
	};

	using UR = bstone::UniqueResource<int*, Deleter>;
	auto value = 100;
	const auto ur = UR{&value};
	tester.check(!(nullptr == ur), "eoae7a2oaxd7c1zx");
}

// bool operator!=(nullptr, const UniqueResource&)
// Not equal.
void test_l9ljlcg6tjrtnftq()
{
	struct Deleter
	{
		void operator()(int*) const noexcept {}
	};

	using UR = bstone::UniqueResource<int*, Deleter>;
	auto value = 100;
	const auto ur = UR{&value};
	tester.check(nullptr != ur, "l9ljlcg6tjrtnftq");
}

// bool operator!=(nullptr, const UniqueResource&)
// Not equal.
void test_xz0dosjcxxysxldj()
{
	struct Deleter
	{
		void operator()(int*) const noexcept {}
	};

	using UR = bstone::UniqueResource<int*, Deleter>;
	const auto ur = UR{};
	tester.check(!(nullptr != ur), "xz0dosjcxxysxldj");
}

// ==========================================================================
// Unique state.

// get_empty_value()
void test_p6zrqv4d6z0djphc()
{
	const auto ref_empty_value = -1;

	struct EmptyValue
	{
		int operator()() const noexcept
		{
			return ref_empty_value;
		}
	};

	struct Deleter
	{
		int dummy;
		void operator()(int) const noexcept {}
	};

	using UR = bstone::UniqueResource<int, Deleter, EmptyValue>;
	const auto empty_value = UR::get_empty_value();
	tester.check(empty_value == ref_empty_value, "p6zrqv4d6z0djphc");
}

// UniqueResource(Deleter)
void test_985uouwe9v1rl9di()
{
	const auto ref_empty_value = -1;

	struct EmptyValue
	{
		int operator()() const noexcept
		{
			return ref_empty_value;
		}
	};

	struct Deleter
	{
		int dummy;
		void operator()(int) const noexcept {}
	};

	using UR = bstone::UniqueResource<int, Deleter, EmptyValue>;
	const auto ur = UR{Deleter{}};
	tester.check(ur.get_empty_value() == ref_empty_value, "985uouwe9v1rl9di");
}

// UniqueResource(Resource, Deleter)
void test_pnd40pwxnek91ts6()
{
	const auto ref_value = 100;
	const auto ref_empty_value = -1;

	struct EmptyValue
	{
		int operator()() const noexcept
		{
			return ref_empty_value;
		}
	};

	struct Deleter
	{
		int dummy;
		void operator()(int) const noexcept {}
	};

	using UR = bstone::UniqueResource<int, Deleter, EmptyValue>;
	const auto ur = UR{ref_value, Deleter{}};
	tester.check(ur.get() == ref_value, "pnd40pwxnek91ts6");
}

// UniqueResource(UniqueResource&&)
void test_s5j39m48t33wijqz()
{
	const auto ref_value = 100;
	const auto ref_empty_value = -1;

	struct EmptyValue
	{
		int operator()() const noexcept
		{
			return ref_empty_value;
		}
	};

	struct Deleter
	{
		int* counter;

		void operator()(int) const noexcept
		{
			++(*counter);
		}
	};

	auto counter = 0;
	auto is_equals = false;

	{
		using UR = bstone::UniqueResource<int, Deleter, EmptyValue>;
		auto ur = UR{UR{ref_value, Deleter{&counter}}};
		is_equals = ur.get() == ref_value;
	}

	tester.check(is_equals && counter == 1, "s5j39m48t33wijqz");
}

// UniqueResource& operator=(UniqueResource&&)
void test_dk4ur020c1a6awal()
{
	const auto ref_value = 100;
	const auto ref_empty_value = -1;

	struct EmptyValue
	{
		int operator()() const noexcept
		{
			return ref_empty_value;
		}
	};

	struct Deleter
	{
		int* counter;

		void operator()(int) const noexcept
		{
			++(*counter);
		}
	};

	auto counter = 0;
	auto is_equal = false;

	{
		using UR = bstone::UniqueResource<int, Deleter, EmptyValue>;
		auto deleter = Deleter{&counter};
		auto ur1 = UR{ref_value, deleter};
		auto ur = UR{deleter};
		ur = std::move(ur1);
		is_equal = ur.get() == ref_value;
	}

	tester.check(is_equal && counter == 1, "dk4ur020c1a6awal");
}

// get()
void test_5jt65pw5ulvjjo5n()
{
	const auto ref_empty_value = -1;

	struct EmptyValue
	{
		int operator()() const noexcept
		{
			return ref_empty_value;
		}
	};

	struct Deleter
	{
		int dummy;

		void operator()(int) const noexcept {}
	};

	using UR = bstone::UniqueResource<int, Deleter, EmptyValue>;
	const auto ur = UR{Deleter{}};
	tester.check(ur.get() == ref_empty_value, "5jt65pw5ulvjjo5n");
}

// is_empty()
void test_p003g90hg66pw7mq()
{
	const auto ref_empty_value = -1;

	struct EmptyValue
	{
		int operator()() const noexcept
		{
			return ref_empty_value;
		}
	};

	struct Deleter
	{
		int dummy;

		void operator()(int) const noexcept {}
	};

	using UR = bstone::UniqueResource<int, Deleter, EmptyValue>;
	const auto ur = UR{Deleter{}};
	tester.check(ur.is_empty(), "p003g90hg66pw7mq");
}

// reset()
void test_xebw87tpiepur40n()
{
	const auto ref_empty_value = -1;

	struct EmptyValue
	{
		int operator()() const noexcept
		{
			return ref_empty_value;
		}
	};

	struct Deleter
	{
		int* counter;

		void operator()(int) const noexcept
		{
			++(*counter);
		}
	};

	auto counter = 0;
	auto is_equal = false;

	{
		using UR = bstone::UniqueResource<int, Deleter, EmptyValue>;
		auto ur = UR{100, Deleter{&counter}};
		ur.reset();
		is_equal = ur.get() == ref_empty_value;
	}

	tester.check(is_equal && counter == 1, "xebw87tpiepur40n");
}

// reset(Resource)
void test_yla859q4iwun5ndz()
{
	const auto ref_initial_value = 100;
	const auto ref_value = 1000;
	const auto ref_empty_value = -1;

	struct EmptyValue
	{
		int operator()() const noexcept
		{
			return ref_empty_value;
		}
	};

	struct Deleter
	{
		int* counter;

		void operator()(int) const noexcept
		{
			++(*counter);
		}
	};

	auto counter = 0;
	auto is_equal = false;

	{
		using UR = bstone::UniqueResource<int, Deleter, EmptyValue>;
		auto ur = UR{ref_initial_value, Deleter{&counter}};
		ur.reset(ref_value);
		is_equal = ur.get() == ref_value;
	}

	tester.check(is_equal && counter == 2, "yla859q4iwun5ndz");
}

// release()
void test_b4mq7z8bda8vf6o9()
{
	const auto ref_value = 100;
	const auto ref_empty_value = -1;

	struct EmptyValue
	{
		int operator()() const noexcept
		{
			return ref_empty_value;
		}
	};

	struct Deleter
	{
		int* counter;

		void operator()(int) const noexcept
		{
			++(*counter);
		}
	};

	auto counter = 0;
	auto is_equal = false;

	{
		using UR = bstone::UniqueResource<int, Deleter, EmptyValue>;
		auto ur = UR{ref_value, Deleter{&counter}};
		const auto value = ur.release();
		is_equal = ur.get() == ref_empty_value && value == ref_value;
	}

	tester.check(is_equal && counter == 0, "b4mq7z8bda8vf6o9");
}

// swap(UniqueResource&)
void test_e286jvzfcguikz4o()
{
	const auto ref_value = 1000;
	const auto ref_empty_value = -1;

	struct EmptyValue
	{
		int operator()() const noexcept
		{
			return ref_empty_value;
		}
	};

	struct Deleter
	{
		int dummy;

		void operator()(int) const noexcept {}
	};

	using UR = bstone::UniqueResource<int, Deleter, EmptyValue>;
	auto ur1 = UR{ref_value, Deleter{}};
	auto ur = UR{Deleter{}};
	ur1.swap(ur);

	tester.check(ur1.get() == ref_empty_value && ur.get() == ref_value, "e286jvzfcguikz4o");
}

// operator*()
void test_btbygrlgqri9qcah()
{
	const auto ref_value = 1000;

	struct Deleter
	{
		int dummy;

		void operator()(int*) const noexcept {}
	};

	using UR = bstone::UniqueResource<int*, Deleter>;
	auto value = ref_value;
	const auto ur = UR{&value, Deleter{}};
	const auto result = *ur;

	tester.check(result == ref_value && ur.get() == &value, "btbygrlgqri9qcah");
}

// operator->()
void test_ugkra91u87gsxmtc()
{
	const auto ref_value = 1000;

	struct Value
	{
		int x;
	};

	struct Deleter
	{
		int dummy;

		void operator()(Value*) const noexcept {}
	};

	using UR = bstone::UniqueResource<Value*, Deleter>;
	auto value = Value{ref_value};
	auto ur = UR{&value, Deleter{}};
	const auto result = ur->x;

	tester.check(result == ref_value && ur.get() == &value, "ugkra91u87gsxmtc");
}

// bool operator==(const UniqueResource&, const UniqueResource&)
// Equal.
void test_whbtdqhs18xl7vfx()
{
	const auto ref_value = 100;

	struct Deleter
	{
		int dummy;

		void operator()(int) const noexcept {}
	};

	using UR = bstone::UniqueResource<int, Deleter>;
	const auto ur1 = UR{ref_value, Deleter{}};
	const auto ur2 = UR{ref_value, Deleter{}};
	tester.check(ur1 == ur2, "whbtdqhs18xl7vfx");
}

// bool operator==(const UniqueResource&, const UniqueResource&)
// Not equal.
void test_bdd5l5d1tu3cqsoh()
{
	const auto ref_value1 = 100;
	const auto ref_value2 = 200;

	struct Deleter
	{
		int dummy;

		void operator()(int) const noexcept {}
	};

	using UR = bstone::UniqueResource<int, Deleter>;
	const auto ur1 = UR{ref_value1, Deleter{}};
	const auto ur2 = UR{ref_value2, Deleter{}};
	tester.check(!(ur1 == ur2), "bdd5l5d1tu3cqsoh");
}

// bool operator!=(const UniqueResource&, const UniqueResource&)
// Not equal.
void test_uugqeeqzltxvf8r9()
{
	const auto ref_value1 = 100;
	const auto ref_value2 = 200;

	struct Deleter
	{
		int dummy;

		void operator()(int) const noexcept {}
	};

	using UR = bstone::UniqueResource<int, Deleter>;
	const auto ur1 = UR{ref_value1, Deleter{}};
	const auto ur2 = UR{ref_value2, Deleter{}};
	tester.check(ur1 != ur2, "uugqeeqzltxvf8r9");
}

// bool operator!=(const UniqueResource&, const UniqueResource&)
// Equal.
void test_1cb2yc9y9ws4f8vb()
{
	const auto ref_value = 100;

	struct Deleter
	{
		int dummy;

		void operator()(int) const noexcept {}
	};

	using UR = bstone::UniqueResource<int, Deleter>;
	const auto ur1 = UR{ref_value, Deleter{}};
	const auto ur2 = UR{ref_value, Deleter{}};
	tester.check(!(ur1 != ur2), "1cb2yc9y9ws4f8vb");
}

// bool operator==(const UniqueResource&, nullptr)
// Equal.
void test_b1tv8cp0a69aovcw()
{
	struct Deleter
	{
		int dummy;

		void operator()(int*) const noexcept {}
	};

	using UR = bstone::UniqueResource<int*, Deleter>;
	const auto ur = UR{Deleter{}};
	tester.check(ur == nullptr, "b1tv8cp0a69aovcw");
}

// bool operator==(const UniqueResource&, nullptr)
// Not equal.
void test_bmlp49xhueryqml1()
{
	struct Deleter
	{
		int dummy;

		void operator()(int*) const noexcept {}
	};

	using UR = bstone::UniqueResource<int*, Deleter>;
	auto value = 100;
	const auto ur = UR{&value, Deleter{}};
	tester.check(!(ur == nullptr), "bmlp49xhueryqml1");
}

// bool operator!=(const UniqueResource&, nullptr)
// Not equal.
void test_raykix8gy7d2i6bb()
{
	struct Deleter
	{
		int dummy;

		void operator()(int*) const noexcept {}
	};

	using UR = bstone::UniqueResource<int*, Deleter>;
	auto value = 100;
	const auto ur = UR{&value, Deleter{}};
	tester.check(ur != nullptr, "raykix8gy7d2i6bb");
}

// bool operator!=(const UniqueResource&, nullptr)
// Not equal.
void test_mukw46ts1mohy0zb()
{
	struct Deleter
	{
		int dummy;

		void operator()(int*) const noexcept {}
	};

	using UR = bstone::UniqueResource<int*, Deleter>;
	const auto ur = UR{Deleter{}};
	tester.check(!(ur != nullptr), "mukw46ts1mohy0zb");
}

// bool operator==(nullptr, const UniqueResource&)
// Equal.
void test_qlcixthuj04lyzea()
{
	struct Deleter
	{
		int dummy;

		void operator()(int*) const noexcept {}
	};

	using UR = bstone::UniqueResource<int*, Deleter>;
	const auto ur = UR{Deleter{}};
	tester.check(nullptr == ur, "qlcixthuj04lyzea");
}

// bool operator==(nullptr, const UniqueResource&)
// Not equal.
void test_72voiayfzhz9xb7w()
{
	struct Deleter
	{
		int dummy;

		void operator()(int*) const noexcept {}
	};

	using UR = bstone::UniqueResource<int*, Deleter>;
	auto value = 100;
	const auto ur = UR{&value, Deleter{}};
	tester.check(!(nullptr == ur), "72voiayfzhz9xb7w");
}

// bool operator!=(nullptr, const UniqueResource&)
// Not equal.
void test_7s4u6jwrjf1s8zo8()
{
	struct Deleter
	{
		int dummy;

		void operator()(int*) const noexcept {}
	};

	using UR = bstone::UniqueResource<int*, Deleter>;
	auto value = 100;
	const auto ur = UR{&value, Deleter{}};
	tester.check(nullptr != ur, "7s4u6jwrjf1s8zo8");
}

// bool operator!=(nullptr, const UniqueResource&)
// Not equal.
void test_9mpqp5y78cl8fdw0()
{
	struct Deleter
	{
		int dummy;

		void operator()(int*) const noexcept {}
	};

	using UR = bstone::UniqueResource<int*, Deleter>;
	const auto ur = UR{Deleter{}};
	tester.check(!(nullptr != ur), "9mpqp5y78cl8fdw0");
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_unique_resource_mono();
		register_unique_resource_unique();
	}

private:
	void register_unique_resource_mono()
	{
		tester.register_test("UniqueResource#5vsgrh0ngiztibku", test_5vsgrh0ngiztibku);
		tester.register_test("UniqueResource#nwhuwglw2a6jo9d6", test_nwhuwglw2a6jo9d6);
		tester.register_test("UniqueResource#p21v8j553fgt2lal", test_p21v8j553fgt2lal);
		tester.register_test("UniqueResource#tf5z5j5up4apbh0b", test_tf5z5j5up4apbh0b);
		tester.register_test("UniqueResource#0q6l99sxup31qxq9", test_0q6l99sxup31qxq9);
		tester.register_test("UniqueResource#nuixo0tzra25rt5f", test_nuixo0tzra25rt5f);
		tester.register_test("UniqueResource#5yyqy7ylaj5upttc", test_5yyqy7ylaj5upttc);
		tester.register_test("UniqueResource#i0gki9p89yl7xv1i", test_i0gki9p89yl7xv1i);
		tester.register_test("UniqueResource#aa2twi6yj4dm1mvo", test_aa2twi6yj4dm1mvo);
		tester.register_test("UniqueResource#qezkdligeaj4qjaq", test_qezkdligeaj4qjaq);
		tester.register_test("UniqueResource#ri9ibhitw8i5tnle", test_ri9ibhitw8i5tnle);
		tester.register_test("UniqueResource#58aiayzra6imm0p8", test_58aiayzra6imm0p8);
		tester.register_test("UniqueResource#fp3toubop8s4t805", test_fp3toubop8s4t805);
		tester.register_test("UniqueResource#iow0ep4j9jhr0s40", test_iow0ep4j9jhr0s40);
		tester.register_test("UniqueResource#895zkfoibw1c5eu0", test_895zkfoibw1c5eu0);
		tester.register_test("UniqueResource#884glztjux6fffet", test_884glztjux6fffet);
		tester.register_test("UniqueResource#ck0h8bfcyrwo4amu", test_ck0h8bfcyrwo4amu);
		tester.register_test("UniqueResource#tcppddfvbsfxx8g7", test_tcppddfvbsfxx8g7);
		tester.register_test("UniqueResource#rr3h438f26wnkhgy", test_rr3h438f26wnkhgy);
		tester.register_test("UniqueResource#rln6v214wkr5bij1", test_rln6v214wkr5bij1);
		tester.register_test("UniqueResource#q6biwiux463ru724", test_q6biwiux463ru724);
		tester.register_test("UniqueResource#0z0gmpop34d4d8ma", test_0z0gmpop34d4d8ma);
		tester.register_test("UniqueResource#eoae7a2oaxd7c1zx", test_eoae7a2oaxd7c1zx);
		tester.register_test("UniqueResource#l9ljlcg6tjrtnftq", test_l9ljlcg6tjrtnftq);
		tester.register_test("UniqueResource#xz0dosjcxxysxldj", test_xz0dosjcxxysxldj);
	}

	void register_unique_resource_unique()
	{
		tester.register_test("UniqueResource#p6zrqv4d6z0djphc", test_p6zrqv4d6z0djphc);
		tester.register_test("UniqueResource#985uouwe9v1rl9di", test_985uouwe9v1rl9di);
		tester.register_test("UniqueResource#pnd40pwxnek91ts6", test_pnd40pwxnek91ts6);
		tester.register_test("UniqueResource#s5j39m48t33wijqz", test_s5j39m48t33wijqz);
		tester.register_test("UniqueResource#dk4ur020c1a6awal", test_dk4ur020c1a6awal);
		tester.register_test("UniqueResource#5jt65pw5ulvjjo5n", test_5jt65pw5ulvjjo5n);
		tester.register_test("UniqueResource#p003g90hg66pw7mq", test_p003g90hg66pw7mq);
		tester.register_test("UniqueResource#xebw87tpiepur40n", test_xebw87tpiepur40n);
		tester.register_test("UniqueResource#yla859q4iwun5ndz", test_yla859q4iwun5ndz);
		tester.register_test("UniqueResource#b4mq7z8bda8vf6o9", test_b4mq7z8bda8vf6o9);
		tester.register_test("UniqueResource#e286jvzfcguikz4o", test_e286jvzfcguikz4o);
		tester.register_test("UniqueResource#btbygrlgqri9qcah", test_btbygrlgqri9qcah);
		tester.register_test("UniqueResource#ugkra91u87gsxmtc", test_ugkra91u87gsxmtc);
		tester.register_test("UniqueResource#whbtdqhs18xl7vfx", test_whbtdqhs18xl7vfx);
		tester.register_test("UniqueResource#bdd5l5d1tu3cqsoh", test_bdd5l5d1tu3cqsoh);
		tester.register_test("UniqueResource#uugqeeqzltxvf8r9", test_uugqeeqzltxvf8r9);
		tester.register_test("UniqueResource#1cb2yc9y9ws4f8vb", test_1cb2yc9y9ws4f8vb);
		tester.register_test("UniqueResource#b1tv8cp0a69aovcw", test_b1tv8cp0a69aovcw);
		tester.register_test("UniqueResource#bmlp49xhueryqml1", test_bmlp49xhueryqml1);
		tester.register_test("UniqueResource#raykix8gy7d2i6bb", test_raykix8gy7d2i6bb);
		tester.register_test("UniqueResource#mukw46ts1mohy0zb", test_mukw46ts1mohy0zb);
		tester.register_test("UniqueResource#qlcixthuj04lyzea", test_qlcixthuj04lyzea);
		tester.register_test("UniqueResource#72voiayfzhz9xb7w", test_72voiayfzhz9xb7w);
		tester.register_test("UniqueResource#7s4u6jwrjf1s8zo8", test_7s4u6jwrjf1s8zo8);
		tester.register_test("UniqueResource#9mpqp5y78cl8fdw0", test_9mpqp5y78cl8fdw0);
	}
};

auto registrator = Registrator{};

} // namespace
