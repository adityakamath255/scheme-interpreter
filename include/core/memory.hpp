#pragma once
#include <core/types.hpp>
#include <vector>

namespace Scheme {

inline HeapEntity*
try_get_heap_entity(Obj& obj) {
  return std::visit(Overloaded{
    [](bool) -> HeapEntity* {
      return nullptr;
    },
    [](double) -> HeapEntity* {
      return nullptr;
    },
    [](char) -> HeapEntity* {
      return nullptr;
    },
    [](Symbol&) -> HeapEntity* {
      return nullptr;
    },
    [](Null) -> HeapEntity* {
      return nullptr;
    },
    [](Void) -> HeapEntity* {
      return nullptr;
    },
    [](String *w) -> HeapEntity* {
      return w;
    },
    [](Cons* ls) -> HeapEntity* {
      return ls;
    },
    [](Vector* v) -> HeapEntity* {
      return v;
    },
    [](Procedure* p) -> HeapEntity* {
      return p;
    },
    [](Builtin* p) -> HeapEntity* {
      return p;
    },
  }, obj);
}

class Allocator {
private:
  std::vector<HeapEntity*> live_memory;
  void mark(const std::vector<HeapEntity*>&);
  void sweep(); 

public:
  Allocator(): live_memory {} {};

  template<typename T, typename... Args>
  T* spawn(Args&&... args) {
    static_assert(std::is_base_of_v<HeapEntity, T>, "attempt to allocate an object not derived from HeapEntity");
    T* obj = new T(std::forward<Args>(args)...);
    live_memory.push_back(obj);
    return obj;
  }

  void recycle();
  void recycle(const std::vector<HeapEntity*>&);
};

}

