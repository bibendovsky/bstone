#ifndef BSTONE_NOT_NULL_INCLUDED
#define BSTONE_NOT_NULL_INCLUDED


namespace bstone
{

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

template<
	typename TException,
	typename T
>
T* not_null(
	T* pointer)
{
	if (pointer != nullptr)
	{
		return pointer;
	}

	throw TException{};
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone


#endif // !BSTONE_NOT_NULL_INCLUDED
