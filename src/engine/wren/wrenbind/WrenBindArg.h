//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef WREN_BIND_ARG_H
#define WREN_BIND_ARG_H

#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
#include "wren.h"
#ifdef __cplusplus
}
#endif // __cplusplus
#include "platform.h"
#include "wrenbind/include/Runtime.h"
#include "wrenbind/include/Arg.h"
#include "wrenbind/include/Alloc.h"
#include "wren/wrenbind/WrenBindAlloc.h"
#include "wrenbind/include/Class.h"
#include "utils/UtlVector.h"
#include "utils/UtlMap.h"
#include "renderer/material/Material.h"
#include "ecs/Entity.h"
#include "ecs/EntityComponent.h"
#include "ecs/components/Camera.h"
#include "ecs/components/Light.h"
#include "ecs/components/Mesh.h"
#include "ecs/components/Transform.h"

class CFixedString;
class CVector4;
class CVector3;
class CVector2;
class CMatrix4;
class CColor;

template<>
struct CWrenBindArg<CFixedString>
{
	static CFixedString Get(WrenVM* pVM, int32 slot)
	{
		return CFixedString(wrenGetSlotString(pVM, slot));
	}

	static void Set(WrenVM* pVM, const CFixedString& v)
	{
		wrenSetSlotString(pVM, 0, v.Get());
	}

	static void SetAt(WrenVM* pVM, int32 slot, const CFixedString& v)
	{
		wrenSetSlotString(pVM, slot, v.Get());
	}
};

template<>
struct CWrenBindArg<CVector2>
{
	static CVector2 Get(WrenVM* pVM, int32 slot)
	{
		void* pData = wrenGetSlotForeign(pVM, slot);
		if (pData)
		{
			Handle_t h = *static_cast<Handle_t*>(pData);
			if (h) return *static_cast<CVector2*>(HandleToPtr(h));
		}

		return CVector2(
			static_cast<float32>(wrenGetSlotDouble(pVM, slot)),
			static_cast<float32>(wrenGetSlotDouble(pVM, slot + 1)));
	}

	static void Set(WrenVM* pVM, const CVector2& v)
	{
		SetAt(pVM, 0, v);
	}

	static void SetAt(WrenVM* pVM, int32 slot, const CVector2& v)
	{
		WrenHandle* pClass = CWrenBindClassOf<CVector2>::Get(pVM);
		if (!pClass)
		{
			wrenSetSlotNull(pVM, slot);

			return;
		}

		wrenEnsureSlots(pVM, slot + 1);
		wrenSetSlotHandle(pVM, slot, pClass);
		wrenSetSlotNewForeign(pVM, slot, slot, sizeof(Handle_t));

		void* pMem = CWrenBindAllocator<CVector2>::Alloc(sizeof(CVector2));
		if (pMem)
		{
			*static_cast<Handle_t*>(wrenGetSlotForeign(pVM, slot)) =
				PtrToHandle(new(pMem) CVector2(v));
		}
	}
};

template<>
struct CWrenBindArg<CVector3>
{
	static CVector3 Get(WrenVM* pVM, int32 slot)
	{
		void* pData = wrenGetSlotForeign(pVM, slot);
		if (pData)
		{
			Handle_t h = *static_cast<Handle_t*>(pData);
			if (h) return *static_cast<CVector3*>(HandleToPtr(h));
		}

		return CVector3(
			static_cast<float32>(wrenGetSlotDouble(pVM, slot)),
			static_cast<float32>(wrenGetSlotDouble(pVM, slot + 1)),
			static_cast<float32>(wrenGetSlotDouble(pVM, slot + 2)));
	}

	static void Set(WrenVM* pVM, const CVector3& v)
	{
		SetAt(pVM, 0, v);
	}

	static void SetAt(WrenVM* pVM, int32 slot, const CVector3& v)
	{
		WrenHandle* pClass = CWrenBindClassOf<CVector3>::Get(pVM);
		if (!pClass)
		{
			wrenSetSlotNull(pVM, slot);

			return;
		}

		wrenEnsureSlots(pVM, slot + 1);
		wrenSetSlotHandle(pVM, slot, pClass);
		wrenSetSlotNewForeign(pVM, slot, slot, sizeof(Handle_t));

		void* pMem = CWrenBindAllocator<CVector3>::Alloc(sizeof(CVector3));
		if (pMem)
		{
			*static_cast<Handle_t*>(wrenGetSlotForeign(pVM, slot)) =
				PtrToHandle(new(pMem) CVector3(v));
		}
	}
};

template<>
struct CWrenBindArg<CVector4>
{
	static CVector4 Get(WrenVM* pVM, int32 slot)
	{
		return CVector4(
			static_cast<float32>(wrenGetSlotDouble(pVM, slot)),
			static_cast<float32>(wrenGetSlotDouble(pVM, slot + 1)),
			static_cast<float32>(wrenGetSlotDouble(pVM, slot + 2)),
			static_cast<float32>(wrenGetSlotDouble(pVM, slot + 3)));
	}

	static void Set(WrenVM* pVM, const CVector4& v)
	{
		wrenSetSlotDouble(pVM, 0, static_cast<double>(v.m_X));
		wrenSetSlotDouble(pVM, 1, static_cast<double>(v.m_Y));
		wrenSetSlotDouble(pVM, 2, static_cast<double>(v.m_Z));
		wrenSetSlotDouble(pVM, 3, static_cast<double>(v.m_W));
	}

	static void SetAt(WrenVM* pVM, int32 slot, const CVector4& v)
	{
		wrenSetSlotDouble(pVM, slot, static_cast<double>(v.m_X));
		wrenSetSlotDouble(pVM, slot + 1, static_cast<double>(v.m_Y));
		wrenSetSlotDouble(pVM, slot + 2, static_cast<double>(v.m_Z));
		wrenSetSlotDouble(pVM, slot + 3, static_cast<double>(v.m_W));
	}
};

template<>
struct CWrenBindArg<CMatrix4>
{
	static CMatrix4 Get(WrenVM* pVM, int32 slot)
	{
		void* pData = wrenGetSlotForeign(pVM, slot);
		if (pData)
		{
			Handle_t h = *static_cast<Handle_t*>(pData);
			if (h) return *static_cast<CMatrix4*>(HandleToPtr(h));
		}

		return CMatrix4();
	}

	static void Set(WrenVM* pVM, const CMatrix4& v)
	{
		SetAt(pVM, 0, v);
	}

	static void SetAt(WrenVM* pVM, int32 slot, const CMatrix4& v)
	{
		WrenHandle* pClass = CWrenBindClassOf<CMatrix4>::Get(pVM);
		if (!pClass)
		{
			wrenSetSlotNull(pVM, slot);

			return;
		}

		wrenEnsureSlots(pVM, slot + 1);
		wrenSetSlotHandle(pVM, slot, pClass);
		wrenSetSlotNewForeign(pVM, slot, slot, sizeof(Handle_t));

		void* pMem = CWrenBindAllocator<CMatrix4>::Alloc(sizeof(CMatrix4));
		if (pMem)
		{
			*static_cast<Handle_t*>(wrenGetSlotForeign(pVM, slot)) =
				PtrToHandle(new(pMem) CMatrix4(v));
		}
	}
};

template<>
struct CWrenBindArg<CColor>
{
	static CColor Get(WrenVM* pVM, int32 slot)
	{
		void* pData = wrenGetSlotForeign(pVM, slot);
		if (pData)
		{
			Handle_t h = *static_cast<Handle_t*>(pData);
			if (h) return *static_cast<CColor*>(HandleToPtr(h));
		}

		return CColor(
			static_cast<float32>(wrenGetSlotDouble(pVM, slot)),
			static_cast<float32>(wrenGetSlotDouble(pVM, slot + 1)),
			static_cast<float32>(wrenGetSlotDouble(pVM, slot + 2)));
	}

	static void Set(WrenVM* pVM, const CColor& v)
	{
		SetAt(pVM, 0, v);
	}

	static void SetAt(WrenVM* pVM, int32 slot, const CColor& v)
	{
		WrenHandle* pClass = CWrenBindClassOf<CColor>::Get(pVM);
		if (!pClass)
		{
			wrenSetSlotNull(pVM, slot);

			return;
		}

		wrenEnsureSlots(pVM, slot + 1);
		wrenSetSlotHandle(pVM, slot, pClass);
		wrenSetSlotNewForeign(pVM, slot, slot, sizeof(Handle_t));

		void* pMem = CWrenBindAllocator<CColor>::Alloc(sizeof(CColor));
		if (pMem)
		{
			*static_cast<Handle_t*>(wrenGetSlotForeign(pVM, slot)) =
				PtrToHandle(new(pMem) CColor(v));
		}
	}
};

template<typename T>
struct CWrenBindArg<CRef<T> >
{
	static void Set(WrenVM* pVM, const CRef<T>& v)
	{
		SetAt(pVM, 0, v);
	}

	static void SetAt(WrenVM* pVM, int32 slot, const CRef<T>& v)
	{
		WrenHandle* pClass = CWrenBindClassOf<T>::Get(pVM);
		if (!v.IsValid() || !pClass)
		{
			wrenSetSlotNull(pVM, slot);

			return;
		}

		wrenEnsureSlots(pVM, slot + 1);
		wrenSetSlotHandle(pVM, slot, pClass);
		wrenSetSlotNewForeign(pVM, slot, slot, sizeof(Handle_t));
		*static_cast<Handle_t*>(wrenGetSlotForeign(pVM, slot)) =
			PtrToHandle(v.Get());

		v->AddRef();
	}
};

template<>
struct CWrenBindArg<CRef<CEntityComponent> >
{
	static void Set(WrenVM* pVM, const CRef<CEntityComponent>& v)
	{
		SetAt(pVM, 0, v);
	}

	static void SetAt(
		WrenVM* pVM,
		int32 slot,
		const CRef<CEntityComponent>& v)
	{
		if (!v.IsValid())
		{
			wrenSetSlotNull(pVM, slot);

			return;
		}

		if (v->IsType("CTransform"))
		{
			Wrap<CTransform>(pVM, slot, v);
		}
		else if (v->IsType("CCamera"))
		{
			Wrap<CCamera>(pVM, slot, v);
		}
		else if (v->IsType("CMesh"))
		{
			Wrap<CMesh>(pVM, slot, v);
		}
		else if (v->IsType("CLight"))
		{
			Wrap<CLight>(pVM, slot, v);
		}
		else
		{
			wrenSetSlotNull(pVM, slot);
		}
	}

private:
	template<typename T>
	static void Wrap(WrenVM* pVM, int32 slot, const CRef<CEntityComponent>& v)
	{
		CWrenBindArg<CRef<T> >::SetAt(
			pVM,
			slot,
			CRef<T>(static_cast<T*>(v.Get())));
	}
};

template<>
struct CWrenBindArg<MaterialProperty_t>
{
	static void Set(WrenVM* pVM, const MaterialProperty_t& p)
	{
		SetAt(pVM, 0, p);
	}

	static void SetAt(WrenVM* pVM, int32 slot, const MaterialProperty_t& p)
	{
		wrenSetSlotNewList(pVM, slot);
		wrenEnsureSlots(pVM, slot + 2);
		int32 valSlot = slot + 1;
		wrenSetSlotDouble(pVM, valSlot, static_cast<double>(p.m_Type));
		wrenInsertInList(pVM, slot, -1, valSlot);
		wrenSetSlotDouble(pVM, valSlot, static_cast<double>(p.m_Value.m_X));
		wrenInsertInList(pVM, slot, -1, valSlot);
		wrenSetSlotDouble(pVM, valSlot, static_cast<double>(p.m_Value.m_Y));
		wrenInsertInList(pVM, slot, -1, valSlot);
		wrenSetSlotDouble(pVM, valSlot, static_cast<double>(p.m_Value.m_Z));
		wrenInsertInList(pVM, slot, -1, valSlot);
		wrenSetSlotDouble(pVM, valSlot, static_cast<double>(p.m_Value.m_W));
		wrenInsertInList(pVM, slot, -1, valSlot);
	}
};

template<>
struct CWrenBindArg<CEntity>
{
	static CEntity Get(WrenVM* pVM, int32 slot)
	{
		return CEntity(
			static_cast<uint32>(wrenGetSlotDouble(pVM, slot)),
			CF_NULL);
	}

	static void Set(WrenVM* pVM, const CEntity& v)
	{
		wrenSetSlotDouble(pVM, 0, static_cast<double>(v.GetID()));
	}

	static void SetAt(WrenVM* pVM, int32 slot, const CEntity& v)
	{
		wrenSetSlotDouble(pVM, slot, static_cast<double>(v.GetID()));
	}
};

template<typename T>
static void WrenBindListReadInto(WrenVM* pVM, int32 slot, CUtlVector<T>& out)
{
	int32 count = wrenGetListCount(pVM, slot);
	wrenEnsureSlots(pVM, slot + 2);
	for (int32 i = 0; i < count; i++)
	{
		wrenGetListElement(pVM, slot, i, slot + 1);
		out.AddToTail(CWrenBindArg<T>::Get(pVM, slot + 1));
	}
}

template<typename T>
struct CWrenBindArg<CUtlVector<T> >
{
	static CUtlVector<T> Get(WrenVM* pVM, int32 slot)
	{
		CUtlVector<T> vec;
		WrenBindListReadInto<T>(pVM, slot, vec);

		return vec;
	}

	static void Set(WrenVM* pVM, const CUtlVector<T>& v)
	{
		SetAt(pVM, 0, v);
	}

	static void SetAt(WrenVM* pVM, int32 slot, const CUtlVector<T>& v)
	{
		wrenSetSlotNewList(pVM, slot);
		wrenEnsureSlots(pVM, slot + 2);
		for (int32 i = 0; i < v.Count(); i++)
		{
			CWrenBindArg<T>::SetAt(pVM, slot + 1, v[i]);
			wrenInsertInList(pVM, slot, -1, slot + 1);
		}
	}
};

template<typename K, typename V>
struct CWrenBindArg<CUtlMap<K, V> >
{
	static CUtlMap<K, V> Get(WrenVM* pVM, int32 slot)
	{
		CUtlMap<K, V> m;
		int32 count = wrenGetListCount(pVM, slot);
		int32 kslot = slot + 1;
		int32 vslot = slot + 2;
		wrenEnsureSlots(pVM, slot + 3);

		for (int32 i = 0; i < count; i += 2)
		{
			wrenGetListElement(pVM, slot, i, kslot);
			wrenGetListElement(pVM, slot, i + 1, vslot);
			m.Insert(
				CWrenBindArg<K>::Get(pVM, kslot),
				CWrenBindArg<V>::Get(pVM, vslot));
		}

		return m;
	}

	static void Set(WrenVM* pVM, const CUtlMap<K, V>& v)
	{
		SetAt(pVM, 0, v);
	}

	static void SetAt(WrenVM* pVM, int32 slot, const CUtlMap<K, V>& v)
	{
		wrenSetSlotNewList(pVM, slot);
		wrenEnsureSlots(pVM, slot + 3);

		for (int32 i = 0; i < v.Count(); i++)
		{
			int32 kslot = slot + 1;
			int32 vslot = slot + 2;
			CWrenBindArg<K>::SetAt(pVM, kslot, v.Key(i).Get());
			CWrenBindArg<V>::SetAt(pVM, vslot, v.Element(i));
			wrenInsertInList(pVM, slot, -1, kslot);
			wrenInsertInList(pVM, slot, -1, vslot);
		}
	}
};

#endif // WREN_BIND_ARG_H