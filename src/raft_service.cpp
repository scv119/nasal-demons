// Copyright (c) 2017, Chen Shen
// All rights reserved.

#include "raft_service.h"

namespace ndemons {

RaftService::RaftService(int64_t nodeId, GroupCongig config)
    : state_(nodeId, config), stateLock_, server_, peers_, watcher_ {}

grpc::Status RaftService::RequestVote(grpc::ServerContext *context,
                                      const VoteRequest *request,
                                      VoteResponse *response) {
  std::lock_guard<std::mutex>(stateLock_);
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
  std::lock_guard<std::mutex>(stateLock_);
}
} // namespace ndemons