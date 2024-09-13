#pragma once

template<typename _T>
struct DataMap
{
	struct ItemDesc
	{
		int offset;
		int size;
	};
	using T = _T;

	struct DescList
	{
		const DataMap<T>::ItemDesc* begin() const { return _begin; }
		const DataMap<T>::ItemDesc* end() const { return _end; }

		const DataMap<T>::ItemDesc* _begin;
		const DataMap<T>::ItemDesc* _end;
	};

	static DescList GetItemDescMap();
	static size_t Size;
};

template<typename T>
static size_t _DataMapSize()
{
	auto dl = DataMap<T>::GetItemDescMap();
	return dl._end - dl._begin;
}

#define BEGIN_DATAMAP( Type ) \
	size_t DataMap<Type>::Size = _DataMapSize<Type>(); \
	DataMap<Type>::DescList DataMap<Type>::GetItemDescMap(){ \
		static ItemDesc itemsDesc[] = { 

#define END_DATAMAP	{ 0, 0 } }; return DescList{ &itemsDesc[0], &itemsDesc[sizeof(itemsDesc) / sizeof(itemsDesc[0])]}; };

#define DATA_FIELD( x ) { offsetof(T, x), sizeof(T::x) },

#define DECLARE_DATAMAP( Type )