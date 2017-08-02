// Copyright (c) 2017, Chen Shen
// All rights reserved.

#pragma once

#include <string>

namespace ndemons {

// An abstract write command to be implemented by raft client.
struct WriteCommand {
  WriteCommand() = default;
  // Deserialize from serialized data.
  WriteCommand(std::string serialized){};
  virtual std::string Serialize() const;
  // Commit the command into underlining system.
  virtual void Commit() const;
  virtual ~WriteCommand() = default;
};
} // namespace ndemons