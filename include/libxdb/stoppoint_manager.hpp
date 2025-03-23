#ifndef XDB_STOPPOINT_MANAGER_HPP
#define XDB_STOPPOINT_MANAGER_HPP

#include <algorithm>
#include <memory>
#include <optional>
#include <vector>
#include "libxdb/types.hpp"
namespace xdb {

template<class T>
class stoppoint_manager {
 public:
  stoppoint_manager() = default;
  stoppoint_manager(const stoppoint_manager&) = delete;
  stoppoint_manager& operator=(const stoppoint_manager&) = delete;
  ~stoppoint_manager() = default;


  void add_stoppoint(T stoppoint) {
    stoppoints_.emplace_back(stoppoint);
  }
  T& push(std::unique_ptr<T> stoppoint) {
    stoppoints_.emplace_back(std::move(stoppoint));
    return *stoppoints_.back();
  }

  bool contains(virt_addr addr) const {
    for (auto& stoppoint : stoppoints_) {
      if (stoppoint->address() == addr) {
        return true;
      }
    }
    return false;
  }
  bool contains(typename T::id_t id) const {
    for (auto& stoppoint : stoppoints_) {
      if (stoppoint->id() == id) {
        return true;
      }
    }
    return false;
  }

  std::optional<T&> get_by_id(typename T::id_t id) const {
    for (auto& stoppoint : stoppoints_) {
      if (stoppoint->id() == id) {
        return *stoppoint; 
      }
    }
    return nullptr;
  }
  std::optional<T&> get_by_address(virt_addr addr) const {
    for (auto& stoppoint : stoppoints_) {
      if (stoppoint->address() == addr) {
        return *stoppoint;
      }
    }
    return nullptr;
  }

 private:
  using SP = std::vector<std::unique_ptr<T>>;

  auto find_by_id(typename T::id_t id) {
    return std::find_if(stoppoints_.begin(), stoppoints_.end(), [id](const auto& stoppoint) {
      return stoppoint->id() == id;
    });
  }

  // auto find_by_id(typename T::id_t id) const { return 
  // }
  struct x {
    std::vector<int> v;
    auto f() {
      return v.begin();
    }
    auto f() const {
      return f();
    }
  };

  SP stoppoints_;
};

}

#endif