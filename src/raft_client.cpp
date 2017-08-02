// Copyright (c) 2017, Chen Shen
// All rights reserved.

#include "raft_client.h"
#include <grpc/support/log.h>

namespace ndemons {
RaftClient::RaftClient(std::shared_ptr<grpc::Channel> channel)
    : stub_(RaftRpc::NewStub(channel)) {}

std::future<VoteResponse> RaftClient::requestVote(VoteRequest request) {
  return std::async(
      std::launch::async, [ this, request = std::move(request) ]() {
        VoteResponse response;
        grpc::CompletionQueue cq;
        grpc::Status status;
        grpc::ClientContext context;
        std::unique_ptr<grpc::ClientAsyncResponseReader<VoteResponse>> rpc(
            stub_->AsyncRequestVote(&context, request, &cq));
        rpc->Finish(&response, &status, (void *)1);
        void *got_tag;
        bool ok = false;
        GPR_ASSERT(cq.Next(&got_tag, &ok));
        GPR_ASSERT(got_tag == (void *)1);
        GPR_ASSERT(ok);
        if (status.ok()) {
          return response;
        }
        throw std::runtime_error("VoteRequest failed.");
      });
}

std::future<AppendResponse> RaftClient::appendEntries(AppendRequest request) {
  return std::async(
      std::launch::async, [ this, request = std::move(request) ]() {
        AppendResponse response;
        grpc::CompletionQueue cq;
        grpc::Status status;
        grpc::ClientContext context;
        std::unique_ptr<grpc::ClientAsyncResponseReader<AppendResponse>> rpc(
            stub_->AsyncAppendEntries(&context, request, &cq));
        rpc->Finish(&response, &status, (void *)1);
        void *got_tag;
        bool ok = false;
        GPR_ASSERT(cq.Next(&got_tag, &ok));
        GPR_ASSERT(got_tag == (void *)1);
        GPR_ASSERT(ok);
        if (status.ok()) {
          return response;
        }
        throw std::runtime_error("Append entries request failed.");
      });
}

} // namespace ndemons