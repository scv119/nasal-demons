// Copyright (c) 2017, Chen Shen
// All rights reserved.

#pragma once

#include <future>
#include <memory>
#include <string>

#include "../build/gen/raft.grpc.pb.h"
#include "../build/gen/raft.pb.h"
#include <grpc++/grpc++.h>

#include "state.h"

namespace ndemons {
class RaftClient {
public:
  explicit RaftClient(std::shared_ptr<grpc::Channel> channel);
  std::future<VoteResponse> requestVote(VoteRequest);
  std::future<AppendResponse> appendEntries(AppendRequest);

private:
  std::unique_ptr<RaftRpc::Stub> stub_;
};
} // namespace ndemons