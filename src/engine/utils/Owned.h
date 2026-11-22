//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef OWNED_H
#define OWNED_H

#pragma once

#include "platform.h"
#include "utils/UtlMemory.h"
#include <new>

template<typename T>
class COwned
{
public:
	COwned() :
		m_pObject(CF_NULL)
	{
	}

	~COwned()
	{
		Destroy();
	}

	void Destroy()
	{
		if (m_pObject)
		{
			m_pObject->~T();
			CUtlMemory::Free(m_pObject);

			m_pObject = CF_NULL;
		}
	}
private:
	COwned(const COwned&);
	COwned& operator=(const COwned&);

	void* Allocate()
	{
		Destroy();

		return CUtlMemory::Alloc(sizeof(T));
	}

	bool Take(T* pObject)
	{
		m_pObject = pObject;

		return true;
	}
public:
	bool Create()
	{
		void* pMemory = Allocate();

		return pMemory && Take(new(pMemory) T());
	}

	template<typename A1>
	bool Create(const A1& arg1)
	{
		void* pMemory = Allocate();

		return pMemory && Take(new(pMemory) T(arg1));
	}

	template<typename A1, typename A2>
	bool Create(const A1& arg1, const A2& arg2)
	{
		void* pMemory = Allocate();

		return pMemory && Take(new(pMemory) T(arg1, arg2));
	}

	T* Get() const
	{
		return m_pObject;
	}

	T* operator->() const
	{
		return m_pObject;
	}

	T& operator*() const
	{
		return *m_pObject;
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

#endif // OWNED_H