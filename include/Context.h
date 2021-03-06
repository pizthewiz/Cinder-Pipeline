//
//  Context.h
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 19 Apr 2014.
//  Copyright 2014-2015 Chorded Constructions. All rights reserved.
//

#pragma once

#include "Node.h"
#include "cinder/gl/Fbo.h"

namespace Cinder { namespace Pipeline {

typedef std::shared_ptr<class Context> ContextRef;
typedef std::shared_ptr<class NodePortConnection> NodePortConnectionRef;

class NodePortConnection : public std::enable_shared_from_this<NodePortConnection> {
public:
    static NodePortConnectionRef create(const NodeRef& source, const std::string& sourceKey, const NodeRef& destination, const std::string& destinationKey) {
        return NodePortConnectionRef(new NodePortConnection(source, sourceKey, destination, destinationKey))->shared_from_this();
    }

    inline NodeRef& getSourceNode() {
        return mSourceNode;
    }
    inline std::string& getSourcePortKey() {
        return mSourcePortKey;
    }
    inline NodeRef& getDestinationNode() {
        return mDestinationNode;
    }
    inline std::string& getDestinationPortKey() {
        return mDestinationPortKey;
    }

private:
    NodePortConnection(const NodeRef& source, const std::string& sourceKey, const NodeRef& destination, const std::string& destinationKey) : mSourceNode(source), mSourcePortKey(sourceKey), mDestinationNode(destination), mDestinationPortKey(destinationKey) {}

    NodeRef mSourceNode;
    std::string mSourcePortKey;
    NodeRef mDestinationNode;
    std::string mDestinationPortKey;
};

class Context : public std::enable_shared_from_this<Context> {
public:
    static ContextRef create();
    ~Context();

    // NB: the dependency soulver requires that attachmentCount = max FBOImage input ports for any node + 1
    void setup(const ci::ivec2& size, GLenum colorFormat = GL_RGBA8, int attachmentCount = 3);

    inline std::vector<NodeRef> getNodes() const { return mNodes; }

    template<typename T>
    std::shared_ptr<T> makeNode(T* node) {
        std::shared_ptr<T> result(node);
//        result->setContext(shared_from_this());
        mNodes.push_back(result);
        return result;
    }

    void connectNodes(const NodeRef& sourceNode, const std::string& sourceNodePortKey, const NodeRef& destinationNode, const std::string& destinationNodePortKey);
    void connectNodes(const NodeRef& sourceNode, const NodeRef& destinationNode);

    void disconnectNodes(const NodeRef& sourceNode, const std::string& sourceNodePortKey, const NodeRef& destinationNode, const std::string& destinationNodePortKey);
    void disconnectNodes(const NodeRef& sourceNode, const NodeRef& destinationNode);

    inline std::map<std::string, NodePortConnectionRef> getInputConnectionsForNode(const NodeRef& node) {
        return mInputConnections[node];
    }
    std::vector<NodePortConnectionRef> getInputConnectionsForNodeWithPortType(const NodeRef& node, NodePortType type) {
        std::vector<NodePortConnectionRef> filteredConnections;
        for (auto& kv : mInputConnections[node]) {
            NodePortRef port = node->getInputPortForKey(kv.first);
            if (port->getType() != type) {
                continue;
            }
            filteredConnections.push_back(kv.second);
        }
        return filteredConnections;
    }
    inline NodePortConnectionRef getInputConnectionForNodeWithPortKey(const NodeRef& node, const std::string& portKey) {
        return mInputConnections[node][portKey];
    }
    inline std::map<std::string, std::vector<NodePortConnectionRef>> getOutputConnectionsForNode(const NodeRef& node) {
        return mOutputConnections[node];
    }
    std::vector<NodePortConnectionRef> getOutputConnectionsForNodeWithPortType(const NodeRef& node, const NodePortType type) {
        std::vector<NodePortConnectionRef> filteredConnections;
        for (auto& kv : mOutputConnections[node]) {
            NodePortRef port = node->getOutputPortForKey(kv.first);
            if (port->getType() != type) {
                continue;
            }
            std::vector<NodePortConnectionRef> connections = kv.second;
            filteredConnections.insert(filteredConnections.end(), connections.begin(), connections.end());
        }
        return filteredConnections;
    }
    inline std::vector<NodePortConnectionRef> getOutputConnectionsForNodeWithPortKey(const NodeRef& node, const std::string& portKey) {
        return mOutputConnections[node][portKey];
    }

    std::string serialize();
    bool serialize(const ci::fs::path& path);

    std::deque<std::deque<NodeRef>> renderStackForRenderNode(const NodeRef& node);
    ci::gl::Texture2dRef evaluate(const NodeRef& node);

private:
    Context();

    void connectNodes(const NodeRef& sourceNode, const NodePortRef& sourceNodePort, const NodeRef& destinationNode, const NodePortRef& destinationNodePort);
    void disconnectNodes(const NodeRef& sourceNode, const NodePortRef& sourceNodePort, const NodeRef& destinationNode, const NodePortRef& destinationNodePort);
    void disconnect(const NodePortConnectionRef& connection);

    ci::gl::FboRef mFBO;
    GLenum mColorFormat;
    int mAttachmentCount;
    std::vector<NodeRef> mNodes;
    // {node -> {key -> connection}}
    std::map<NodeRef, std::map<std::string, NodePortConnectionRef>> mInputConnections;
    // {node -> {key -> [connection, ...]}}
    std::map<NodeRef, std::map<std::string, std::vector<NodePortConnectionRef>>> mOutputConnections;

    NodeRef mRenderNode;
    std::deque<std::deque<NodeRef>> mRenderStack;
};

}}
