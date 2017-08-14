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
    : id_(id), raftGroup_(group), role_(Role::FOLLOWER),
      currentTerm_(kDefaultTerm), votedFor_(kNotVoted), logs_(),
      commitedIndex_(kDummyLogIndex), lastApplied_(kDummyLogIndex),
      nextIndex_(), matchedIndex_() {
  logs_.push_back(generateDummyLog());
}

AppendResponse NodeState::ReceiveHeartbeat(AppendRequest request) {
  std::lock_guard<std::mutext> guard(roleMutex_);
  AppendResponse response;

  if (request.term() < currentTerm_) {
    repsonse.set_term(currentTerm_);
    response.set_success(false);
    return repsonse;
  }

  lastHeartBeat_ = std::chrono::system_clock::now();
  response.set_term(request.term());
  currentTerm = request.term();

  if (role_ != Role::FOLLOWER) {
    role_ = Role::FOLLOWER;
    response.set_success(true);
    roleChanged_.notify_one();
  }
  return response;
}

void NodeState::ReceiveVoteResponse(VoteResponse response) {
  std::unique_lock<std::mutex> cLock(candidateStateMutex_);
  if (response.term() != voteTerm_) {
    return;
  }
  voteRecieved_++;
  if (voteRecieved_ < 3) {
    return;
  }

  std::lock_guard<std::mutext> guard(roleMutex_);
  if (role_ != Role::CANDIDATE || currentTerm != voteTerm_ + 1) {
    return;
  }
  voteRecieved_ = 0;
  voteFor_ = 0;
  voteTerm_ = 0;
  currentTerm++;
  role = Role::LEADER;
  return;
}

void NodeState::RunAsFollower() {}

void NodeState::RunAsCandidate() {
  std::unique_lock<std::mutex> lock(roleMutex_);
  if (role_ != Role::CANDIDATE) {
    return;
  }

  std::unique_lock<std::mutex> cLock(candidateStateMutex_);
  voteTerm_ = currentTerm_ + 1;
  voteRecieved_ = 1;
  votedFor = id_;
  VoteRequest request;
  request.set_term(voteTerm_);
  cLock.unlock();

  request.set_candidate_id(id_);
  for (auto &peer : peers_) {
    peer.second->requestVote(request, [
      this,
    ](VoteResponse response) { this->ReceiveVoteResponse(response); });
  }
  auto now = std::chrono::system_clock::now();
  roleChanged_.wait_until(x`, now + voteTimeout_);

  if (role_ != Role::LEADER) {
    return;
  }

  std::unique_lock<std::mutex> cLock1(candidateStateMutex_);
  voteTerm_ = 0;
  voteRecieved_ = 0;
  votedFor = 0;
  cLock.unlock();

  // timeout logic
}

} // namespace ndemons