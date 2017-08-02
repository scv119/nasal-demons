// Copyright (c) 2017, Chen Shen
// All rights reserved.

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "../build/gen/raft.pb.h"

namespace ndemons {

class Log {
public:
  Log(std::string &&payload, int64_t term)
      : payload_(std::move(payload)), term_(term) {}
  Log(const Log &log) = delete;
  Log &operator=(const Log &log) = delete;
  Log(Log &&log) = default;
  Log &operator=(Log &&log) = default;
  ~Log() = default;

  int64_t Term() const { return term_; }
  std::string &Payload() { return payload_; }

private:
  std::string payload_;
  int64_t term_;
};

enum class State {
  FOLLOWER = 0,
  CANDIDATE,
  LEADER,
};

class NodeState {
public:
  NodeState(int64_t id, GroupConfig group);

  NodeState(const NodeState &state) = delete;
  NodeState &operator=(const NodeState &state) = delete;
  NodeState(NodeState &&state) = default;
  NodeState &operator=(NodeState &&state) = default;
  ~NodeState() = default;

  void electedAsLeader() noexcept;

private:
  int64_t id_;
  GroupConfig raftGroup_;
  State state_;

  // Section: Persistent raft state.
  // TODO(chenshen) these states need to be persisted.
  int64_t currentTerm_;
  int64_t votedFor_;
  std::vector<Log> logs_;

  // Section: Volatile raft state.
  std::size_t commitedIndex_;
  std::size_t lastApplied_;

  // Section: Volatile raft states for leaders.
  std::unordered_map<int64_t, std::size_t> nextIndex_;
  std::unordered_map<int64_t, std::size_t> matchedIndex_;
};
} // namespace ndemons