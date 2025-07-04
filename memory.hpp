#pragma once
#include "types.hpp"
#include <vector>

namespace Scheme {

inline HeapEntity*
try_get_heap_entity(const Obj& obj) {
  if (is_pair(obj)) {
    return as_pair(obj);
  }
  else if (is_primitive(obj)) {
    return as_primitive(obj);
  }
  else if (is_procedure(obj)) {
    return as_procedure(obj);
  }
  else if (is_vector(obj)) {
    return as_vector(obj);
  }
  else {
    return nullptr;
  }
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

