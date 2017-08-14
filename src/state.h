// Copyright (c) 2017, Chen Shen
// All rights reserved.

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "../build/gen/raft.pb.h"

namespace ndemons {
extern static int64_t kNotVoted;

class Log {
public:
  Log(std::string &&payload, int64_t term)
      : payload_(std::move(payload)), term_(term) {}

  // Disallow copy.
  Log(const Log &log) = delete;
  Log &operator=(const Log &log) = delete;

  int64_t Term() const { return term_; }
  std::string &Payload() { return payload_; }

private:
  std::string payload_;
  int64_t term_;
};

enum class Role {
  FOLLOWER = 0,
  CANDIDATE,
  LEADER,
};

class NodeState {
public:
  NodeState(int64_t id, GroupConfig group);

  // Disallow copy.
  NodeState(const NodeState &state) = delete;
  NodeState &operator=(const NodeState &state) = delete;

  AppendResponse ReceiveHeartbeat(AppendRequest request);

private:
  void RunAsFollower();

private:
  int64_t id_;
  GroupConfig raftGroup_;
  std::chrono::duration heartBeatTimeout_;
  std::chrono::duration voteTimeout_;

  // Section: Role and Term state.
  // TODO(chenshen) these states need to be persisted.
  std::mutex roleMutex_;
  std::condition_variable roleChanged_;
  int64_t currentTerm_;
  Role role_;

  // Section: Volatile candiate state.
  std::mutex candidateStateMutex_;
  int64_t voteTerm_;
  std::size_t voteRecieved_;
  int64_t votedFor_;

  // Section: Volatile raft state.
  std::atomic<std::chrono::system_clock::time_point> lastHeartBeat_;
  std::size_t commitedIndex_;
  std::size_t lastApplied_;

  // Section: Volatile raft states for leaders.
  std::unordered_map<int64_t, std::size_t> nextIndex_;
  std::unordered_map<int64_t, std::size_t> matchedIndex_;

  // Section: Logs
  std::vector<Log> logs_;

  friend class RaftService;
  friend class RaftWatcher;
};
} // namespace ndemons