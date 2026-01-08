/**
 * MIT License
 *
 * Copyright (c) 2026 Christoffer Gustafsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <dc/types.hpp>

namespace dc
{

// <beginning of c code>

// ========================================================================== //
// Types & Constants
// ========================================================================== //

typedef PmU32 ProbeSequenceLength;

#if !defined(kPmMapDefaultMaxLoadFactor)
#define kPmMapDefaultMaxLoadFactor 0.75f
#endif

#if !defined(kPmMapEntryTombstone)
#define kPmMapEntryTombstone 0
#endif

// ========================================================================== //
// Macros
// ========================================================================== //

/// Hash map with robin hood open addressing collision resolution.
///
/// Macro function is used to declare a map type with the given name,
/// containing values of a specific type. It will declare the map type as well
/// as the functions, but will not implement them.
///
/// \param hashFn Use definition: PmU64 hash(const Key* key);
/// \param compareFn Use definition: PmBool compare(const Key* a, const Key* b);
/// \param keyDestroyFn Use definition: void keyDestroy(Key* key, void*
/// userData);
#define PM_MAP_DECLARE(Name, Key, Value, hashFn, compareFn, keyDestroyFn)      \
  typedef struct Pm##Name##Entry                                               \
  {                                                                            \
    Key key;                                                                   \
    Value value;                                                               \
  } Pm##Name##Entry;                                                           \
                                                                               \
  typedef struct Pm##Name##InternalEntry                                       \
  {                                                                            \
    ProbeSequenceLength probeSequenceLength;                                   \
    Pm##Name##Entry entry;                                                     \
  } Pm##Name##InternalEntry;                                                   \
                                                                               \
  typedef struct Pm##Name                                                      \
  {                                                                            \
    PmAllocator* allocator;                                                    \
    Pm##Name##InternalEntry* data;                                             \
    PmU64 capacity;                                                            \
    PmU64 size;                                                                \
    PmF32 maxLoadFactor;                                                       \
    void* userData;                                                            \
  } Pm##Name;                                                                  \
                                                                               \
  PM_API PM_NODISCARD PmResult pm##Name##Create(PM_IN PmAllocator* allocator,  \
                                                PM_OUT Pm##Name* mapOut,       \
                                                PmU64 capacity,                \
                                                PmF32 maxLoadFactor,           \
                                                PM_IN_OPT void* userData);     \
                                                                               \
  PM_API void pm##Name##Destroy(PM_IN Pm##Name* map);                          \
                                                                               \
  PM_API PM_NODISCARD PmResult pm##Name##Resize(PM_IN Pm##Name* map,           \
                                                PmU64 newCapacity);            \
                                                                               \
  PM_API PM_NODISCARD PM_RESULT PmResult pm##Name##Insert(                     \
    PM_IN Pm##Name* map,                                                       \
    PM_IN const Key* key,                                                      \
    PM_OUT_PTR_MAYBENULL Value** valueOut);                                    \
                                                                               \
  PM_API PM_NODISCARD PM_RETURN_MAYBENULL Pm##Name##Entry* pm##Name##TryGet(   \
    PM_IN Pm##Name* map, PM_IN const Key* key);                                \
                                                                               \
  PM_API PM_NODISCARD PM_RESULT PmResult pm##Name##Remove(                     \
    PM_IN Pm##Name* map, PM_IN const Key* key, PM_OUT_OPT Value* valueOut);

// -------------------------------------------------------------------------- //

/// Macro function that is used to define a map type with the given name,
/// containing values of a specific type. It declare the type and functions as
/// well as implements the functions.
///
/// Note that for a specific name of map this can only occur once in a single
/// compilation unit. Otherwise there will be multiply defined symbols.
#define PM_MAP_DEFINE(Name, Key, Value, hashFn, compareFn, keyDestroyFn)       \
  PM_MAP_DECLARE(Name, Key, Value, hashFn, compareFn, keyDestoryFn)            \
                                                                               \
  PM_API PM_NODISCARD PmResult pm##Name##Create(PM_IN PmAllocator* allocator,  \
                                                PM_OUT Pm##Name* mapOut,       \
                                                PmU64 capacity,                \
                                                PmF32 maxLoadFactor,           \
                                                PM_IN_OPT void* userData)      \
  {                                                                            \
    mapOut->allocator = allocator;                                             \
    mapOut->capacity = 0;                                                      \
    mapOut->size = 0;                                                          \
    mapOut->userData = userData;                                               \
                                                                               \
    if (maxLoadFactor > .9f) {                                                 \
      return kPmResultInvalidArgument;                                         \
    }                                                                          \
    mapOut->maxLoadFactor = maxLoadFactor;                                     \
                                                                               \
    mapOut->data = pmAllocatorAllocateArrayT(                                  \
      mapOut->allocator, Pm##Name##InternalEntry, capacity);                   \
    if (!mapOut->data) {                                                       \
      return kPmResultOutOfMemory;                                             \
    }                                                                          \
    mapOut->capacity = capacity;                                               \
    pmMemoryClear(mapOut->data,                                                \
                  mapOut->capacity * sizeof(Pm##Name##InternalEntry));         \
                                                                               \
    return kPmResultSuccess;                                                   \
  }                                                                            \
                                                                               \
  PM_API void pm##Name##Destroy(PM_IN Pm##Name* map)                           \
  {                                                                            \
    if (map->data) {                                                           \
      pmAllocatorFree(map->allocator, map->data);                              \
    }                                                                          \
  }                                                                            \
                                                                               \
  PM_API PM_NODISCARD PmResult pm##Name##Resize(PM_IN Pm##Name* map,           \
                                                PmU64 newCapacity)             \
  {                                                                            \
    if (newCapacity <= map->capacity) {                                        \
      return kPmResultSuccess;                                                 \
    }                                                                          \
                                                                               \
    Pm##Name##InternalEntry* data = pmAllocatorAllocateArrayT(                 \
      map->allocator, Pm##Name##InternalEntry, newCapacity);                   \
    if (!data) {                                                               \
      return kPmResultOutOfMemory;                                             \
    }                                                                          \
                                                                               \
    pmMemoryClear(data, newCapacity * sizeof(Pm##Name##InternalEntry));        \
                                                                               \
    const PmU64 oldCapacity = map->capacity;                                   \
    const PmU64 oldSize = map->size;                                           \
    Pm##Name##InternalEntry* oldData = map->data;                              \
    map->data = data;                                                          \
    map->size = 0;                                                             \
    map->capacity = newCapacity;                                               \
                                                                               \
    PmResult result = kPmResultSuccess;                                        \
    for (PmU64 i = 0; i < oldCapacity; ++i) {                                  \
      if (oldData[i].probeSequenceLength == kPmMapEntryTombstone) {            \
        continue;                                                              \
      }                                                                        \
                                                                               \
      Value* value = nullptr;                                                  \
      result =                                                                 \
        pm##Name##Insert(map, (const Key*)&oldData[i].entry.key, &value);      \
      if (PM_IS_FAILURE(result)) {                                             \
        break;                                                                 \
      }                                                                        \
      /* TODO(cgustafsson): convince SAL that value cannot be nullptr */       \
      if (value)                                                               \
        pmMemoryCopyT(value, &oldData[i].entry.value, Value, 1);               \
    }                                                                          \
                                                                               \
    if (PM_IS_FAILURE(result)) {                                               \
      /* revert the resizing */                                                \
      map->data = oldData;                                                     \
      map->capacity = oldCapacity;                                             \
      map->size = oldSize;                                                     \
      pmAllocatorFree(map->allocator, data);                                   \
      return result;                                                           \
    }                                                                          \
                                                                               \
    pmAllocatorFree(map->allocator, oldData);                                  \
                                                                               \
    return kPmResultSuccess;                                                   \
  }                                                                            \
                                                                               \
  PM_API PM_NODISCARD PM_RESULT PmResult pm##Name##Insert(                     \
    PM_IN Pm##Name* map,                                                       \
    PM_IN const Key* key,                                                      \
    PM_OUT_PTR_MAYBENULL Value** valueOut)                                     \
  {                                                                            \
    PM_OUT_SET(valueOut, nullptr);                                             \
    if ((PmF32)map->size / (PmF32)map->capacity > map->maxLoadFactor) {        \
      PM_TRY(pm##Name##Resize(map, map->capacity * 2));                        \
    }                                                                          \
                                                                               \
    const PmU64 hash = hashFn(key, map->userData);                             \
    PmU64 bucket = hash % map->capacity;                                       \
                                                                               \
    ProbeSequenceLength probeSequenceLength = 1;                               \
    for (;;) {                                                                 \
      Pm##Name##InternalEntry* entry = &map->data[bucket];                     \
                                                                               \
      if (probeSequenceLength > entry->probeSequenceLength) {                  \
        if (entry->probeSequenceLength == 0) {                                 \
          /* Bucket empty */                                                   \
          map->data[bucket].probeSequenceLength = probeSequenceLength;         \
          pmMemoryCopyT(&map->data[bucket].entry.key, key, Key, 1);            \
          PM_OUT_SET(valueOut, &map->data[bucket].entry.value);                \
          map->size += 1;                                                      \
          break;                                                               \
        }                                                                      \
                                                                               \
        /* Bucket occupied, lets move it. Robin hood! */                       \
        Key tmpKey;                                                            \
        pmMemoryCopyT(&tmpKey, &map->data[bucket].entry.key, Key, 1);          \
        Value tmpValue;                                                        \
        pmMemoryCopyT(&tmpValue, &map->data[bucket].entry.value, Value, 1);    \
                                                                               \
        const ProbeSequenceLength backupOldProbeSequnceLength =                \
          map->data[bucket].probeSequenceLength;                               \
                                                                               \
        map->data[bucket].probeSequenceLength = probeSequenceLength;           \
        pmMemoryCopyT(&map->data[bucket].entry.key, key, Key, 1);              \
        PM_OUT_SET(valueOut, &map->data[bucket].entry.value);                  \
        /* We inserted one, but removed one. No change in size */              \
        /* map->size += 1 - 1; */                                              \
                                                                               \
        Value* insertValue = nullptr;                                          \
        if (PM_IS_FAILURE(                                                     \
              pm##Name##Insert(map, (const Key*)&tmpKey, &insertValue))) {     \
          /* On failure we have to undo the move. */                           \
          map->data[bucket].probeSequenceLength = backupOldProbeSequnceLength; \
          pmMemoryCopyT(&map->data[bucket].entry.key, &tmpKey, Key, 1);        \
          pmMemoryCopyT(&map->data[bucket].entry.value, &tmpValue, Value, 1);  \
        } else {                                                               \
          pmMemoryCopyT(insertValue, &tmpValue, Value, 1);                     \
        }                                                                      \
                                                                               \
        break;                                                                 \
      }                                                                        \
                                                                               \
      /* Bucket occupied and with smaller PSL than ours, try next. */          \
      ++probeSequenceLength;                                                   \
      ++bucket;                                                                \
      if (bucket >= map->capacity) {                                           \
        bucket = 0;                                                            \
      }                                                                        \
    }                                                                          \
                                                                               \
    return kPmResultSuccess;                                                   \
  }                                                                            \
                                                                               \
  PM_API PM_NODISCARD PM_RETURN_MAYBENULL Pm##Name##Entry* pm##Name##TryGet(   \
    PM_IN Pm##Name* map, PM_IN const Key* key)                                 \
  {                                                                            \
    const PmU64 hash = hashFn(key, map->userData);                             \
    PmU64 bucket = hash % map->capacity;                                       \
                                                                               \
    for (;;) {                                                                 \
      if (map->data[bucket].probeSequenceLength == kPmMapEntryTombstone) {     \
        break;                                                                 \
      }                                                                        \
                                                                               \
      if (compareFn(                                                           \
            key, (const Key*)&map->data[bucket].entry.key, map->userData)) {   \
        return &map->data[bucket].entry;                                       \
      }                                                                        \
                                                                               \
      ++bucket;                                                                \
    }                                                                          \
                                                                               \
    return nullptr;                                                            \
  }                                                                            \
                                                                               \
  PM_API PM_NODISCARD PM_RESULT PmResult pm##Name##Remove(                     \
    PM_IN Pm##Name* map, PM_IN const Key* key, PM_OUT_OPT Value* valueOut)     \
  {                                                                            \
    /* TODO cgustafsson: If load factor too small, we should downsize. */      \
    /* if ((PmF32)map->size / (PmF32)map->capacity > map->minLoadFactor) { */  \
    /*  PM_TRY(mapResize(map, map->capacity / 2)); */                          \
    /*} */                                                                     \
                                                                               \
    Pm##Name##Entry* userEntry = pm##Name##TryGet(map, key);                   \
    if (userEntry) {                                                           \
      Pm##Name##InternalEntry* entry =                                         \
        (Pm##Name##InternalEntry*)(((uintptr_t)userEntry) -                    \
                                   sizeof(ProbeSequenceLength));               \
                                                                               \
      entry->probeSequenceLength = kPmMapEntryTombstone;                       \
      PM_OUT_COPY_OPT(valueOut, &userEntry->value);                            \
      keyDestroyFn(&userEntry->key, &map->userData);                           \
      map->size -= 1;                                                          \
      /* If next entry has PSL of larger than 1 we move it and anything */     \
      /* after.*/                                                              \
      PmU64 bucket = ((PmU64)((uintptr_t)entry - (uintptr_t)map->data)) /      \
                     sizeof(Pm##Name##InternalEntry);                          \
      PmU64 prevBucket = bucket;                                               \
      PmU32 moves = 0;                                                         \
      for (;;) {                                                               \
        prevBucket = bucket;                                                   \
        ++bucket;                                                              \
        if (bucket >= map->capacity) {                                         \
          bucket = 0;                                                          \
        }                                                                      \
                                                                               \
        if (map->data[bucket].probeSequenceLength <= 1) {                      \
          break;                                                               \
        }                                                                      \
                                                                               \
        map->data[prevBucket].probeSequenceLength =                            \
          map->data[bucket].probeSequenceLength - 1;                           \
        pmMemoryCopyT(&map->data[prevBucket].entry.key,                        \
                      &map->data[bucket].entry.key,                            \
                      Key,                                                     \
                      1);                                                      \
        pmMemoryCopyT(&map->data[prevBucket].entry.value,                      \
                      &map->data[bucket].entry.value,                          \
                      Value,                                                   \
                      1);                                                      \
        ++moves;                                                               \
      }                                                                        \
                                                                               \
      if (moves > 0) {                                                         \
        /* Don't forget to clear what we moved. */                             \
        map->data[prevBucket].probeSequenceLength = kPmMapEntryTombstone;      \
      }                                                                        \
      return kPmResultSuccess;                                                 \
    }                                                                          \
                                                                               \
    PM_OUT_COPY_OPT(valueOut, &(Value){ 0 });                                  \
                                                                               \
    return kPmResultInvalidArgument;                                           \
  }

// <end of c code>

}
