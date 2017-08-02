// Copyright (c) 2017, Chen Shen
// All rights reserved.

#pragma once

#include "build/gen/raft.pb.h"

namespace ndemons {
struct GroupManager {
  virtual Group getGroup(int64_t id) const;
  virtual Node getLeader(int64_t id) const;
  virtual ~GroupManager(){};
};
} // namespace ndemons