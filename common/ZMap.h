#pragma once
#include "asserts.h"
#include "ZRecyclable.h"

template <typename T, typename U, typename V>
class ZMap
{
public: // the 16 should be a sizeof(something) but idk what it is
	struct _PAIR : ZRecyclable<_PAIR, 16, _PAIR>
	{
		_PAIR* pNext;
		T key;
		U value;

		_PAIR() {
			this->pNext = 0;
			this->key = 0;
			this->value = U();
		}

		_PAIR(T const key, _PAIR* const pNext)
		{
			this->pNext = pNext;
			this->key = key;
			this->value = U();
		}
	};

private:
	_PAIR** m_apTable;
	size_t m_uTableSize;
	size_t m_uCount;
	size_t m_uAutoGrowEvery128;
	size_t m_uAutoGrowLimit;

public:
	ZMap()
	{
		this->m_apTable = nullptr;
		this->m_uTableSize = 0;
		this->m_uCount = 0;
		this->m_uAutoGrowEvery128 = 0;
		this->m_uAutoGrowLimit = 0;
	}

	ZMap(size_t uHashTableSize, size_t uAutoGrowEvery128)
	{
		this->m_apTable = nullptr;
		this->m_uTableSize = uHashTableSize;
		this->m_uCount = 0;

		this->CalcAutoGrow(uAutoGrowEvery128);
	}

	virtual ~ZMap()
	{
		this->RemoveAll();
	}

	_PAIR* GetHeadPosition()
	{
		_PAIR** apTable = this->m_apTable;

		if (!apTable) return nullptr;

		_PAIR** pTableEnd = &apTable[this->m_uTableSize];

		if (apTable >= pTableEnd) return nullptr;

		while (!*apTable)
		{
			apTable += 1;
			if (apTable >= pTableEnd) return nullptr;
		}
		return *apTable;
	}

	_PAIR* GetPos()
	{

	}

	U* GetAt(const T* key)
	{
		ZMap<T, U, V>::_PAIR** v3; // esi
		ZMap<T, U, V>::_PAIR* v5; // esi

		v3 = this->m_apTable;

		if (!v3) return 0;

		v5 = v3[_rotr(*key, 5) % this->m_uTableSize];

		if (!v5) return 0;

		while (v5->key != *key)
		{
			v5 = v5->pNext;

			if (!v5) return 0;
		}

		// we are gonna skip the copying for now
		//if (value) ZRef<AdditionPsd>::operator=(value, &v5->value);

		return &v5->value;
	}

	_PAIR* GetNext()
	{

	}

	_PAIR* Insert()
	{

	}

	void RemoveAll()
	{

	}

	BOOL RemoveKey(const T* key)
	{
		if (!this->m_apTable) return false;

		_PAIR* pEntry = this->m_apTable[rotr(*key, 5) % this->m_uTableSize];

		if (!pEntry) return false;

		while (pEntry->key != *key)
		{
			if (!pEntry->pNext) return false;

			pEntry = pEntry->pNext;
		}

		// this probably wont work unless you hook the memory allocation/deallocation
		// methods that maple uses.. itll give a wrong stack free error
		delete pEntry;

		this->m_uCount -= 1;
		return true;
	}

private:
	void ResizeHashTable(size_t uHashTableSize, size_t uAutoGrowEvery128)
	{

	}

	void CalcAutoGrow(size_t uAutoGrowEvery128)
	{
		if (uAutoGrowEvery128)
		{
			this->m_uAutoGrowEvery128 = uAutoGrowEvery128;
		}
		if (this->m_uAutoGrowEvery128 == -1)
		{
			this->m_uAutoGrowLimit = -1;
		}
		else
		{
			this->m_uAutoGrowLimit = this->m_uAutoGrowEvery128 * this->m_uTableSize >> 7;
		}
	}
};

assert_size(sizeof(ZMap<long, long, long>::_PAIR), 0x10)
assert_size(sizeof(ZMap<long, long, long>), 0x18)