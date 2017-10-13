// Copyright (c) 2017, Chen Shen
// All rights reserved.

#include "state.h"

namespace ndemons {

Node::Node(int64_t id, std::vector<Peer> peers,
           std::shared_ptr<boost::asio::io_service> io)
    : id_(id) role_(Role::FOLLOWER), currentTerm_(0), votedFor_(0), mutex(),
      timer_(), io_(io), peers_(peers) {}

void Node::start() { BecomeFollower(); }

void Node::BecomeFollower() {
  std::lock_guard<std::mutex> lock(mutex_);
  role_ = Role::FOLLOWER;
  votedFor_ = 0;
  SetFollowerTimer();
}

void Node::BecomeCandidate() {
  std::lock_guard<std::mutex> lock(mutex_);
  currentTerm_++;
  votedFor_ = id_;
  role_ = Role::CANDIDATE;
  RequestVote();
  SetCandidateTimer();
}

void Node::BecomeLeader() {
  std::lock_guard<std::mutex> lock(mutex_);
  role_ = Role::LEADER;
  SetLeaderTimer();
}

void Node::HandleMessage(Message m) {
  std::lock_guard<std::mutex> lock(mutex_);
  switch (role_) {
  case Role::FOLLOWER:
    return HandleFollowerMessage(m);
  case Role::CANDIDATE:
    return HandleCandidateMessage(m);
  default:
    return HandleLeaderMessage(m);
  }
}

void Node::HandleFollowerMessage(Message m) {
  switch (m.type()) {
  case MessageType::HEARTBEAT_REQ:
  case MessageType::APPEND_REQ:
    if (m.term() < currentTerm_) {
      return SendAppendResponse(false);
    }
    lastHeardFromLeader_ = boost::posix_time::second_clock::local_time();
    if (m.term() > currentTerm_) {
      currentTerm_ = m.term();
      leaderId_ = m.leader_id();
    }
    // TODO: implement the logs logic.
    SendAppendResponse(true);
    break;
  case MessageType::VOTE_REQ:
    if (m.term() >= currentTerm_ && (votedFor_ == 0 || votedFor_ == m.candidate_id()) {
      votedFor_ = m.from_id();
      return SendVoteResponse(true);
    }
    // TODO: implement the logs logic.
    return SendVoteResponse(false);
    break;
  default:
    // Do nothing for other messages.
  }
}

void Node::HandleCandidateMessage(Message m) {}

void Node::HandleLeaderMessage(Message m) {}

void Node::HandleSendHeartBeatTimer() {}
void Node::HandleHeartBeatTimeout() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (/* not timeout */) {
    SetFollowerTimer();
  } else {
    BecomeCandidate();
  }
}
void Node::HandleVoteTimeout() {}
void Node::SetFollowerTimer() {}
void Node::SetCandidateTimer() {}
void Node::SetLeaderTimer() {}
void Node::CancelTimer() {}

void Node::SendHeartBeat() {}
void Node::RequestVote() {}
void Node::SendMessage(Message m) {}

} // namespace ndemons