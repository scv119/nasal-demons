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
  void ConnectToPeers();
  void DisconnectFromPeers();
  void StartRaftWatcher();
  void StopRaftWatcher();
  void GetRoleAndTerm(Role &role, int64_t &term) noexcept const;
  int64_t GetTerm() noexcept const;
  bool TryPromoteToCandidate() noexcept;

private:
  NodeState state_;

  // Lock to ensure read/write to state_ is atomic.
  std::recursive_mutex stateLock_;
  std::unique_ptr<grpc::Server> server_;
  std::unordered_map<int64_t, std::unique_ptr<RaftClient>> peers_;

  // Thread watch the state of the node and change state between
  // Candidate/Leader/Follower when criteria met.
  std::unique_ptr<std::thread> raftWatcherThread_;
  friend class RaftWatcher;
  std::shared_ptr<RaftWatcher> raftWatcher_;
};

class RaftWatcher {
public:
  RaftWatcher(RaftService *service);

  // Disable copy.
  RaftWatcher(const RaftWatcher &) = delete;
  RaftWatcher &operator=(const RaftWatcher &) = delete;

  void Start();
  void Stop();

private:
  void Run();
  void AsFollower();

private:
  std::atomic<bool> started_;
  std::atomic<bool> roleChanged_;
  RaftService *service_;
};

} // namespace ndemons