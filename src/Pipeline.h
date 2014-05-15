//
//  Pipeline.h
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 19 Apr 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#pragma once

#include "Node.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Fbo.h"

namespace Cinder { namespace Pipeline {

typedef std::shared_ptr<class Pipeline> PipelineRef;
typedef std::shared_ptr<class Branch> BranchRef;
typedef std::shared_ptr<class Connection> BranchConnectionRef;

class Connection : public std::enable_shared_from_this<Connection> {
public:
    static BranchConnectionRef create(const BranchRef& source, const BranchRef& destination = nullptr, unsigned int cost = 0) {
        return BranchConnectionRef(new Connection(source, destination, cost))->shared_from_this();
    }

    BranchRef& getSourceBranch() {
        return mSourceBranch;
    }
    BranchRef& getDestinationBranch() {
        return mDestinationBranch;
    }
    Connection& setCost(unsigned int cost) {
        mCost = cost;
        return *this;
    }
    unsigned int getCost() const {
        return mCost;
    }

private:
    Connection(const BranchRef& source, const BranchRef& destination, unsigned int cost) : mSourceBranch(source), mDestinationBranch(destination), mCost(cost) {}

    BranchRef mSourceBranch;
    BranchRef mDestinationBranch;
    unsigned int mCost;
};

class Branch : public std::enable_shared_from_this<Branch> {
public:
    static BranchRef create() { return BranchRef(new Branch())->shared_from_this(); }
    ~Branch() {}

    void setNodes(std::deque<NodeRef>& nodes) { mNodes = nodes; }
    const std::deque<NodeRef>& getNodes() const { return mNodes; }

    void connectInputBranch(const BranchRef& branch) {
        mInputConnections.push_back(Connection::create(branch, nullptr, branch->getMaxInputCost()));

        auto result = std::max_element(mInputConnections.begin(), mInputConnections.end(), [](const BranchConnectionRef& c1, const BranchConnectionRef& c2) {
            return c1->getCost() < c2->getCost();
        });
        unsigned int cost = mInputConnections.at(std::distance(mInputConnections.begin(), result))->getCost();
        setMaxInputCost(cost + (unsigned int)mInputConnections.size());

//        branch->connectOutputBranch(shared_from_this());
    }

    std::vector<BranchConnectionRef>& getInputConnections() { return mInputConnections; }
//    std::vector<BranchRef>& getOutputConnections() { return mOutputConnections; }

    void setMaxInputCost(unsigned int cost) { mMaxInputCost = cost; }
    unsigned int getMaxInputCost() const { return mMaxInputCost; }

private:
    Branch() : mMaxInputCost(0) {}

//    void connectOutputBranch(const BranchRef& branch) { mOutputBranches.push_back(Connection::create(branch)); }

    std::deque<NodeRef> mNodes;
    std::vector<BranchConnectionRef> mInputConnections;
//    std::vector<BranchConnectionRef> mOutputConnections;
    unsigned int mMaxInputCost;
};

class Pipeline : public std::enable_shared_from_this<Pipeline> {
public:
    static PipelineRef create();
    ~Pipeline();

    void setup(const Vec2i size);

    gl::Texture& evaluate(const NodeRef& node);

private:
    Pipeline();

    BranchRef branchForNode(const NodeRef& node);
    std::deque<BranchRef> renderStackForRootBranch(const BranchRef& branch);

    gl::Fbo mFBO;
};

}}
