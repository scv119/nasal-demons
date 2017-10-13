// Copyright (c) 2017, Chen Shen
// All rights reserved.

#pragma once

#include "boost/asio.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "message.pb.h"
#include "peer.h"
#include <memory>
#include <mutex>
#include <vector>

namespace ndemons {

enum class Role {
  FOLLOWER = 0,
  CANDIDATE,
  LEADER,
};

class Node {
public:
  Node(int64_t id, std::vector<Peer> peers,
       std::shared_ptr<boost::asio::io_service> io);
  Node(const Node &) = delete;
  Node &operator=(const Node &) = delete;
  Node(Node &&node) = default;
  Node &operator=(Node &&node) = default;

  void start();

private:
  void BecomeFollower();
  void BecomeCandidate();
  void BecomeLeader();

  void HandleMessage(Message m);
  void HandleFollowerMessage(Message m);
  void HandleCandidateMessage(Message m);
  void HandleLeaderMessage(Message m);

  void HandleSendHeartBeatTimer();
  void HandleHeartBeatTimeout();
  void HandleVoteTimeout();

  void SetFollowerTimer();
  void SetCandidateTimer();
  void SetLeaderTimer();
  void CancelTimer();

  void SendHeartBeat();
  void RequestVote();
  void SendMessage(Message m);
  void SendAppendResponse(bool success);
  void SendVoteResponse(bool success);

private:
  int64_t id_;
  Role role_;
  int64_t currentTerm_;
  int64_t votedFor_;
  int64_t leaderId_;

  std::mutex mutex_;
  std::unique_ptr<boost::asio::deadline_timer> timer_;
  std::shared_ptr<boost::asio::io_service> io_;

  std::vector<Peer> peers_;

  boost::posix_time::duration followerTimeout_;
  boost::posix_time::duration voteTimeout_;
  boost::posix_time::duration sendHeartbeatInterval_;
  boost::posix_time::ptime lastHeardFromLeader_;
}

} // namespace ndemons