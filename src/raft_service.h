// Copyright (c) 2017, Chen Shen
// All rights reserved.

#pragma once

#include <memory>
#include <mutex>
#include <thread>

#include "../build/gen/raft.grpc.pb.h"
#include "../build/gen/raft.pb.h"
#include <grpc++/grpc++.h>

#include "raft_client.h"
#include "state.h"

namespace ndemons {
class RaftService final : RaftRpc::Service {
public:
  RaftService(int64_t nodeId, GroupConfig group);

  // Disable copy.
  RaftService(const RaftService &) = delete;
  RaftService &operator=(const RaftService &) = delete;

  // Rpc handlers.
  grpc::Status RequestVote(grpc::ServerContext *context,
                           const VoteRequest *request, VoteResponse *response);
  grpc::Status AppendEntries(grpc::ServerContext *context,
                             const AppendRequest *request,
                             AppendResponse *response);

  // Join raft group and serving traffic.
  void Start();

  // Stop serving traffic.
  void Stop();

private:
  void ResetTimeout();

private:
  NodeState state_;

  // Lock to ensure read/write to state_ is atomic.
  std::mutex stateLock_;
  std::unique_ptr<grpc::Server> server_;
  std::unordered_map<int64_t, std::unique_ptr<RaftClient>> peers_;

  // Thread watch the state of the node and change state between
  // Candidate/Leader/Follower when criteria met.
  std::thread raftWatcher_;

  // Thread responsible for listening rpc calls from peers.
  std::thread rpcServer_;
};

} // namespace ndemons