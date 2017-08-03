// Copyright (c) 2017, Chen Shen
// All rights reserved.

#include "state.h"

namespace ndemons {
static int64_t kDefaultTerm = 1;
static int64_t kNotVoted = 0;
static std::size_t kDummyLogIndex = 0;

namespace {
Log generateDummyLog() { return Log("", 0); }
} // namespace

NodeState::NodeState(int64_t id, GroupConfig group)
    : id_(id), raftGroup_(group), state_(State::FOLLOWER),
      currentTerm_(kDefaultTerm), votedFor_(kNotVoted), logs_(),
      commitedIndex_(kDummyLogIndex), lastApplied_(kDummyLogIndex),
      nextIndex_(), matchedIndex_() {
  logs_.push_back(generateDummyLog());
}

void NodeState::electedAsLeader() noexcept {
  assert(state_ != State::LEADER);
  state_ = State::LEADER;
  nextIndex_.clear();
  matchedIndex_.clear();
  for (auto &nodeConfig : raftGroup_.nodes()) {
    nextIndex_[nodeConfig.id()] = logs_.size() + 1;
    matchedIndex_[nodeConfig.id()] = kDummyLogIndex;
  }
}
} // namespace ndemons