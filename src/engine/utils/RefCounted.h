//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef REF_COUNTED_H
#define REF_COUNTED_H

#pragma once

#include "platform.h"
#include "utils/UtlMemory.h"
#include "utils/UtlVector.h"

class CRefCounted
{
public:
	CRefCounted() :
		m_Refs(1)
	{
	}

	// A copy is a new object, it does not share the source references
	CRefCounted(const CRefCounted&) :
		m_Refs(1)
	{
	}

	CRefCounted& operator=(const CRefCounted&)
	{
		return *this;
	}

	virtual ~CRefCounted()
	{
	}

	void AddRef()
	{
		m_Refs++;
	}

	void Release()
	{
		m_Refs--;

		if (m_Refs == 0)
		{
			this->~CRefCounted();
			CUtlMemory::Free(this);
		}
	}

	static void SafeAddRef(CRefCounted* pObject)
	{
		if (pObject)
		{
			pObject->AddRef();
		}
	}

	static void SafeRelease(CRefCounted* pObject)
	{
		if (pObject)
		{
			pObject->Release();
		}
	}

	static void* operator new(size_t, void* pMemory)
	{
		return pMemory;
	}

	static void operator delete(void*, void*)
	{
	}

	static void operator delete(void* pMemory)
	{
		CUtlMemory::Free(pMemory);
	}
private:
	static void* operator new(size_t);

	uint32 m_Refs;
};

template<typename T>
class CRef
{
public:
	CRef() :
		m_pObject(CF_NULL)
	{
	}

	explicit CRef(T* pObject) :
		m_pObject(pObject)
	{
		CRefCounted::SafeAddRef(m_pObject);
	}

	CRef(const CRef& other) :
		m_pObject(other.m_pObject)
	{
		CRefCounted::SafeAddRef(m_pObject);
	}

	~CRef()
	{
		CRefCounted::SafeRelease(m_pObject);
	}

	// Takes over the reference a new object was created with
	static CRef Adopt(T* pObject)
	{
		CRef ref;
		ref.m_pObject = pObject;

		return ref;
	}

	// Builds an object in CUtlMemory, the result is not valid if that fails
	static CRef Create()
	{
		void* pMemory = CUtlMemory::Alloc(sizeof(T));
		if (!pMemory)
		{
			return CRef();
		}

		return Adopt(new(pMemory) T());
	}

	template<typename A1>
	static CRef Create(const A1& arg1)
	{
		void* pMemory = CUtlMemory::Alloc(sizeof(T));
		if (!pMemory)
		{
			return CRef();
		}

		return Adopt(new(pMemory) T(arg1));
	}

	template<typename A1, typename A2>
	static CRef Create(const A1& arg1, const A2& arg2)
	{
		void* pMemory = CUtlMemory::Alloc(sizeof(T));
		if (!pMemory)
		{
			return CRef();
		}

		return Adopt(new(pMemory) T(arg1, arg2));
	}

	// The new reference is taken first, the object may be the current one
	void Reset(T* pObject = CF_NULL)
	{
		CRefCounted::SafeAddRef(pObject);
		CRefCounted::SafeRelease(m_pObject);

		m_pObject = pObject;
	}

	CRef& operator=(const CRef& other)
	{
		Reset(other.m_pObject);

		return *this;
	}

	T* Get() const
	{
		return m_pObject;
	}

	T* operator->() const
	{
		return m_pObject;
	}

	operator T*() const
	{
		return m_pObject;
	}

	bool IsValid() const
	{
		return m_pObject != CF_NULL;
	}
private:
	T* m_pObject;
};

template<typename T>
class CRefList
{
private:
	void AddRefAll()
	{
		for (int32 i = 0; i < m_Items.Count(); i++)
		{
			CRefCounted::SafeAddRef(m_Items[i]);
		}
	}

public:
	CRefList()
	{
	}

	~CRefList()
	{
		RemoveAll();
	}

	void RemoveAll()
	{
		for (int32 i = 0; i < m_Items.Count(); i++)
		{
			CRefCounted::SafeRelease(m_Items[i]);
		}

		m_Items.RemoveAll();
	}

	CRefList(const CRefList& other) :
		m_Items(other.m_Items)
	{
		AddRefAll();
	}

	CRefList& operator=(const CRefList& other)
	{
		// The new references are taken first, the lists may share objects
		CRefList copy(other);

		RemoveAll();
		m_Items = copy.m_Items;
		AddRefAll();

		return *this;
	}

	void Add(T* pObject)
	{
		CRefCounted::SafeAddRef(pObject);

		m_Items.AddToTail(pObject);
	}

	// Grows the list with null elements when index is past the end
	void Set(int32 index, T* pObject)
	{
		while (m_Items.Count() <= index)
		{
			m_Items.AddToTail(CF_NULL);
		}

		CRefCounted::SafeAddRef(pObject);
		CRefCounted::SafeRelease(m_Items[index]);

		m_Items[index] = pObject;
	}

	void Assign(T* pObject)
	{
		CRefCounted::SafeAddRef(pObject);

		RemoveAll();
		m_Items.AddToTail(pObject);
	}

	T* Get(int32 index) const
	{
		return (index >= 0 && index < m_Items.Count())
			? m_Items[index]
			: CF_NULL;
	}

	int32 Count() const
	{
		return m_Items.Count();
	}

	const CUtlVector<T*>& Items() const
	{
		return m_Items;
	}
private:
	CUtlVector<T*> m_Items;
};

#endif // REF_COUNTED_H