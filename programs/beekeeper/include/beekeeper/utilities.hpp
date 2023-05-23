#pragma once

#include <fc/reflect/reflect.hpp>

#include <functional>
#include <string>
#include <chrono>

namespace beekeeper {

struct wallet_details
{
  std::string name;
  bool unlocked = false;
};

struct info
{
  std::string now;
  std::string timeout_time;
};

namespace types
{
  using basic_method_type         = std::function<void()>;
  using notification_method_type  = basic_method_type;
  using lock_method_type          = basic_method_type;
  using timepoint_t               = std::chrono::time_point<std::chrono::system_clock>;
}

}

FC_REFLECT( beekeeper::wallet_details, (name)(unlocked) )
FC_REFLECT( beekeeper::info, (now)(timeout_time) )