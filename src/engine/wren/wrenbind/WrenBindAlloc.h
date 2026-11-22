//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef WREN_BIND_ALLOC_H
#define WREN_BIND_ALLOC_H

#pragma once

#include "wrenbind/include/Alloc.h"
#include "utils/RefCounted.h"
#include "utils/UtlMemory.h"

template<typename T>
void* CWrenBindAllocator<T>::Alloc(size_t size)
{
	return CUtlMemory::Alloc(size);
}

template<typename T>
void WrenBindDispose(T* pObject, const CRefCounted*)
{
	pObject->Release();
}

template<typename T>
void WrenBindDispose(T* pObject, const void*)
{
	pObject->~T();
	CUtlMemory::Free(pObject);
}

template<typename T>
void CWrenBindAllocator<T>::Free(void* p)
{
	if (!p)
	{
		return;
	}

	T* pObject = static_cast<T*>(p);

	WrenBindDispose(pObject, pObject);
}

#endif // WREN_BIND_ALLOC_H