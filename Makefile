HOST_SYSTEM = $(shell uname | cut -f 1 -d_)
SYSTEM ?= $(HOST_SYSTEM)
CXX = g++
CPPFLAGS += -I/usr/local/include -pthread
CXXFLAGS += -std=c++1y
ifeq ($(SYSTEM),Darwin)
LDFLAGS += -L/usr/local/lib `pkg-config --libs grpc++`            \
           -lgrpc++_reflection \
           -lprotobuf -lpthread -ldl
else
LDFLAGS += -L/usr/local/lib `pkg-config --libs grpc++`            \
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed \
           -lprotobuf -lpthread -ldl
endif
PROTOC = protoc
GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`
PROTOS_PATH = ./protos
SRC_PATH = ./src

client.pb:
	$(PROTOC) -I $(PROTOS_PATH) --cpp_out=build/gen  $(PROTOS_PATH)/raft.proto

client.grpc: client.pb
	$(PROTOC) -I $(PROTOS_PATH) --grpc_out=build/gen --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH)  $(PROTOS_PATH)/client.proto

raft.pb:
	$(PROTOC) -I $(PROTOS_PATH) --cpp_out=build/gen  $(PROTOS_PATH)/raft.proto

raft.grpc: raft.pb
	$(PROTOC) -I $(PROTOS_PATH) --grpc_out=build/gen --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH)  $(PROTOS_PATH)/raft.proto

state: raft.pb
	$(CXX) $(CXXFLAGS) -o stats.o $(SRC_PATH)/state.cpp
