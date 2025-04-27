#pragma once
#include "types.hpp"
#include <list>

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
  else {
    return nullptr;
  }
}

class Allocator {
private:
  std::list<HeapEntity*> live_memory;
  void mark(const std::vector<HeapEntity*>&);
  void sweep(); 

public:
  Allocator(): live_memory {} {};

  template<typename T, typename... Args>
  T* make(Args&&... args) {
    static_assert(std::is_base_of_v<HeapEntity, T>, "Allocator can only allocate HeapEntity-derived types");
    T* obj = new T(std::forward<Args>(args)...);
    live_memory.push_back(obj);
    return obj;
  }

  void register_entity(HeapEntity *ent) {live_memory.push_back(ent);}
  void cleanup();
  void cleanup(const std::vector<HeapEntity*>&);
};

}

