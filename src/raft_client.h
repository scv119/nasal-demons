// Copyright (c) 2017, Chen Shen
// All rights reserved.

#pragma once

#include <functional>
#include <memory>
#include <mutex>

#include "../build/gen/raft.grpc.pb.h"
#include "../build/gen/raft.pb.h"
#include <grpc++/grpc++.h>

#include "state.h"

namespace ndemons {
class RaftClient {
public:
  explicit RaftClient(std::shared_ptr<grpc::Channel> channel);

  // Disable copy.
  RaftClient(const RaftClient &) = delete;
  RaftClient &operator=(const RaftClient &) = delete;

  void requestVote(VoteRequest, std::function<void(VoteResponse)>);
  void appendEntries(AppendRequest, std::function<void(AppendResponse)>);

private:
  std::unique_ptr<RaftRpc::Stub> stub_;
};
} // namespace ndemons