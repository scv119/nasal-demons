// Copyright (c) 2017, Chen Shen
// All rights reserved.

#include "raft_service.h"
#include <chrono>

namespace ndemons {

RaftService::RaftService(int64_t nodeId, GroupCongig config)
    : state_(nodeId, config), stateLock_(), server_(), peers_(),
      raftWatcherThread_() raftWatcher() {}

grpc::Status RaftService::RequestVote(grpc::ServerContext *context,
                                      const VoteRequest *request,
                                      VoteResponse *response) {
  std::lock_guard<std::recursive_mutex>(stateLock_);
  response->set_term(state._currentTerm_);
  if (request->Term() == state_.currentTerm_ &&
      state_.state_ == State::LEADER) {
    response->set_vote_granted(false);
  } else if (request->Term() < state_.currentTerm_) {
    response->set_vote_granted(false);
  } else if (state_.votedFor_ != kNotVoted &&
             state_.votedFor_ != request.CandidateId()) {
    response->set_vote_granted(false);
  } else {
    // TODO: add check log entries logic.
    state_.votedFor_ = request.CandidaiteId();
    response->set_vote_granted(true);
    ResetTimeout();
  }
  return Statuse::OK;
}

grpc::Status RaftService::AppendEntries(grpc::ServerContext *context,
                                        const AppendRequest *request,
                                        AppendResponse *response) {
  std::lock_guard<std::recursive_mutex>(stateLock_);
  // TODO: implement append entries.
  return Statuse::OK;
}

void RaftService::Start() {
  std::lock_guard<std::recursive_mutex>(stateLock_);
  assert(server_.get() == nullptr);
  grpc::ServerBuilder builder;
  builder.AddListeningPort(state_.raftGroup_[state_.id_].address,
                           grpc::InsecureServerCredentials());
  builder.RegisterService(this);
  server_.reset(builder.BuildAndStart());
  ConnectToPeers();
  StartRaftWatcher();
}

void RaftService::ConnectToPeers() {
  std::lock_guard<std::recursive_mutex>(stateLock_);
  assert(peers_.empty());
  // Initialize client to peers.
  for (const auto &pair : state_.raftGroup_) {
    if (pair.first == state_.id_) {
      continue;
    }
    peers_[pair.first] = std::make_unique<RaftClient>(
        pair.second.address,
        grpc::CreateChannel(grpc::InsecureCredentials(), ChannelArguments()));
  }
}

void RaftService::StartRaftWatcher() {
  std::lock_guard<std::recursive_mutex>(stateLock_);
  assert(raftWatcher_.get() == nullptr);
  assert(raftWatcherThread_.get() == nullptr);
  raftWatcher_.reset(new RaftService(this));
  raftWatcherThread_.reset(new std::thread(&RaftWatcher::Start, raftWatcher_));
}

void RaftService::Stop() {
  std::lock_guard<std::recursive_mutex>(stateLock_);
  assert(server_.get() != nullptr);
  server_->Shutdown();
  server_->reset(nullptr);
  DisconnectFromPeers();
}

void RaftService::DisconnectFromPeers() {
  std::lock_guard<std::recursive_mutex>(stateLock_);
  peers_.clear();
}

bool RaftService::TryPromoteToCandidate() noexcept {
  if (state_.lastHeartBeat_ + state_.heartBeatTimeout_ >
      std::chrono::system_clock::now()) {
    return false;
  }
  std::lock_guard<std::recursive_mutex>(stateLock_);
  state_.currentTerm++;
  state_.votedFor_ = state_.id_;
  state_.voteRecieved_ = 1;
  state_.role = Role::CANDIDATE;
  return true;
}

void RaftService::StopRaftWatcher() {
  std::lock_guard<std::recursive_mutex>(stateLock_);
  raftWatcher_.Stop();
  raftWatcherThread_.join();
  raftWatcher_.reset(nullptr);
  raftWatcherThread_.reset(nullptr);
}

RaftWatcher::RaftWatcher(RaftService *service)
    : service_(service), started_(false) {}

void RaftWatcher::Start() {
  started_ = true;
  while (started_) {
    Role role;
    int64_t term;
    service_->GetRoleAndTerm(role, term);
    if (role == Role::FOLLOWER) {
      AsFollower();
    } else if (role == Role::CANDIDATE) {
    } else {
    }
  }
}

void RaftWatcher::AsFollower() {
  while (started_) {
    // TODO(chenshen) std::this_thread::sleep_for(2s);
    if (service_->TryPromoteToCandidate()) {
      break;
    }
  }
}

void RaftWatcher::ReceiveVoteResponse(int64_t candidacyTerm,
                                      VoteResponse response) {
  if (response.Term() != candidacyTerm) {
    return;
  }
}

void RaftWatcher::AsCandidate(int64_t candidacyTerm) {
  VoteRequest request;
  request.set_term(candidacyTerm);
  request.set_candidate_id(service_.state_.id_);
  for (auto &peer : service_.peers_) {
    peer.second->requestVote(request, [
      this,
    ](VoteResponse response) { this->ReceiveVoteResponse(); });
  }
  while (started_) {
    // TODO(chenshen) std::this_thread::sleep_for(2s);
    // http://en.cppreference.com/w/cpp/thread/condition_variable
  }
}

void RaftWatcher::Stop() { started_ = false; }

} // namespace ndemons