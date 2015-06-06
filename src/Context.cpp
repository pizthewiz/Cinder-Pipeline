//
//  Context.cpp
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 19 Apr 2014.
//  Copyright 2014-2015 Chorded Constructions. All rights reserved.
//

#include "Context.h"
#include "SourceNode.h"
#include "EffectorNode.h"
#include "cinder/Utilities.h"
#include "cinder/Json.h"

#include "boost/format.hpp"

using namespace ci;

namespace Cinder { namespace Pipeline {

ContextRef Context::create() {
    return ContextRef(new Context())->shared_from_this();
}

Context::Context() {
}

Context::~Context() {
}

#pragma mark -

void Context::setup(const ivec2& size, GLenum colorFormat, int attachmentCount) {
    // bail if size and attachments are unchanged
    if (mFBO && size == mFBO->getSize() && colorFormat == mColorFormat && attachmentCount == mAttachmentCount) {
        return;
    }

#if defined(DEBUG)
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    cinder::app::console() << "GL_RENDERER: " << renderer << std::endl;
    const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    cinder::app::console() << "GL_VENDOR: " << vendor << std::endl;
    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    cinder::app::console() << "GL_VERSION: " << version << std::endl;
    const char* shadingLanguageVersion = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    cinder::app::console() << "GL_SHADING_LANGUAGE_VERSION: " << shadingLanguageVersion << std::endl;

    cinder::app::console() << "GL_EXTENSIONS: " << std::endl;
    GLint extensionCount = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &extensionCount);
    for (GLint idx = 0; idx < extensionCount; idx++) {
        std::string extension(reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, idx)));
        cinder::app::console() << " " << extension << std::endl;
    }

    GLint texSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);
    cinder::app::console() << "GL_MAX_TEXTURE_SIZE: " << texSize << std::endl;
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &texSize);
    cinder::app::console() << "GL_MAX_3D_TEXTURE_SIZE: " << texSize << std::endl;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texSize);
    cinder::app::console() << "GL_MAX_TEXTURE_IMAGE_UNITS: " << texSize << std::endl;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &texSize);
    cinder::app::console() << "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS: " << texSize << std::endl;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &texSize);
    cinder::app::console() << "GL_MAX_COLOR_ATTACHMENTS: " << texSize << std::endl;
    
    cinder::app::console() << std::string(13, '-') << std::endl;
#endif

//    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);
//    if (size.x > texSize) {
//        // TODO - tile horizontally
//        cinder::app::console() << "ERROR - setup width '" << size.x << "' exceeds maximum texture size '" << texSize << "', tiling unimplemented" << std::endl;
//    }
//    if (size.y > texSize) {
//        // TODO - tile vertically
//        cinder::app::console() << "ERROR - setup height '" << size.y << "' exceeds maximum texture size '" << texSize << "', tiling unimplemented" << std::endl;
//    }
//
//    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &texSize);
//    if (attachments > GL_MAX_COLOR_ATTACHMENTS) {
//        cinder::app::console() << "ERROR - number of attachments '" << attachments << "' exceeds '" << texSize << "'" << std::endl;
//    }
//
//    float attachmentMemorySizeMB = size.x * size.y * 4 / 1024 / 1024;
//    float totalSizeMB = attachmentMemorySizeMB * attachments;
//
//    if (gl::isExtensionAvailable("GL_NV_texture_barrier")) {
//        if (texSize >= 2 * size.x && texSize >= size.y) {
//            // TODO - double-wide
//        } else if (texSize >= 2 * size.y && texSize >= size.x) {
//            // TODO - double-tall
//        } else {
//            // TODO - tile
//        }
//    }

    gl::Fbo::Format format;
    for (unsigned int idx = 0; idx < attachmentCount; idx++) {
        format.attachment(GL_COLOR_ATTACHMENT0 + idx, gl::Texture2d::create(size.x, size.y, gl::Texture2d::Format().internalFormat(colorFormat)));
    }
    mFBO = gl::Fbo::create(size.x, size.y, format);

    GLenum buffers[attachmentCount];
    for (unsigned int idx = 0; idx < attachmentCount; idx++) {
        buffers[idx] = GL_COLOR_ATTACHMENT0 + idx;
    }
    glDrawBuffers(attachmentCount, buffers);
    gl::ScopedMatrices matricies;
    gl::ScopedViewport viewport(ivec2(0), mFBO->getSize());
    gl::ScopedFramebuffer fbo(mFBO);
    gl::clear(ColorAf(0.0, 0.0, 0.0, 0.0));

    mColorFormat = colorFormat;
    mAttachmentCount = attachmentCount;
}

#pragma mark - CONNECTIONS

void Context::connectNodes(const NodeRef& sourceNode, const NodePortRef& sourcePort, const NodeRef& destinationNode, const NodePortRef& destinationPort) {
    // bail if the port types don't match
    if (sourcePort->getType() != destinationPort->getType()) {
        return;
    }

    // remove existing connection if destination port is already has one
    NodePortConnectionRef connection = mInputConnections[destinationNode][destinationPort->getKey()];
    if (connection) {
        disconnect(connection);
    }

    connection = NodePortConnection::create(sourceNode, sourcePort->getKey(), destinationNode, destinationPort->getKey());
    mInputConnections[destinationNode][destinationPort->getKey()] = connection;
    mOutputConnections[sourceNode][sourcePort->getKey()].push_back(connection);

    // wipe render stack to force a rebuild
    mRenderStack.clear();
}

void Context::connectNodes(const NodeRef& sourceNode, const std::string& sourceNodePortKey, const NodeRef& destinationNode, const std::string& destinationNodePortKey) {
    NodePortRef sourcePort = sourceNode->getOutputPortForKey(sourceNodePortKey);
    NodePortRef destinationPort = destinationNode->getInputPortForKey(destinationNodePortKey);
    connectNodes(sourceNode, sourcePort, destinationNode, destinationPort);
}

void Context::connectNodes(const NodeRef& sourceNode, const NodeRef& destinationNode) {
    connectNodes(sourceNode, NodeOutputPortKeyImage, destinationNode, NodeInputPortKeyImage);
}

void Context::disconnect(const NodePortConnectionRef& connection) {
    // NB - erase replaces connection with a nullptr, it does not remove the key
    mInputConnections[connection->getDestinationNode()].erase(connection->getDestinationPortKey());

    std::vector<NodePortConnectionRef> connections = mOutputConnections[connection->getSourceNode()][connection->getSourcePortKey()];
    connections.erase(std::find(connections.begin(), connections.end(), connection));

    // wipe render stack to force a rebuild
    mRenderStack.clear();
}

void Context::disconnectNodes(const NodeRef& sourceNode, const NodePortRef& sourcePort, const NodeRef& destinationNode, const NodePortRef& destinationPort) {
    NodePortConnectionRef connection = mInputConnections[destinationNode][destinationPort->getKey()];

    // make sure nodes are connected on the expected ports
    if (!connection || connection->getSourceNode() != sourceNode || connection->getSourcePortKey() != sourcePort->getKey()) {
        return;
    }

    disconnect(connection);
}

void Context::disconnectNodes(const NodeRef& sourceNode, const std::string& sourceNodePortKey, const NodeRef& destinationNode, const std::string& destinationNodePortKey) {
    NodePortRef sourcePort = sourceNode->getOutputPortForKey(sourceNodePortKey);
    NodePortRef destinationPort = destinationNode->getInputPortForKey(destinationNodePortKey);
    disconnectNodes(sourceNode, sourcePort, destinationNode, destinationPort);
}

void Context::disconnectNodes(const NodeRef& sourceNode, const NodeRef& destinationNode) {
    disconnectNodes(sourceNode, NodeOutputPortKeyImage, destinationNode, NodeInputPortKeyImage);
}

#pragma mark -

BranchRef Context::branchForNode(const NodeRef& node) {
    std::map<NodeRef, BranchRef> branchMap;
    std::deque<NodeRef> nodeStack;

    // create branches
    NodeRef n = node;
    while (n) {
        if (branchMap.count(n) == 0) {
            std::deque<NodeRef> nodes;

            NodeRef n2 = n;
            while (n2) {
                nodes.push_front(n2);

                std::vector<NodePortConnectionRef> connections = getInputConnectionsForNodeWithPortType(n2, NodePortType::FBOImage);
                if (connections.size() == 0) {
                    n2 = nullptr;
                } else if (connections.size() == 1) {
                    NodePortConnectionRef connection = connections.at(0);
                    n2 = connection->getSourceNode();

                    // branch on multiple outputs
                    if (getOutputConnectionsForNodeWithPortType(n2, NodePortType::FBOImage).size() > 1) {
                        nodeStack.push_front(n2);
                        n2 = nullptr;
                    }
                } else if (connections.size() > 1) {
                    // branch on multiple inputs
                    for (const NodePortConnectionRef& c : connections) {
                        nodeStack.push_front(c->getSourceNode());
                    }
                    n2 = nullptr;
                }
            }

            BranchRef branch = Branch::create(nodes);
            branchMap[n] = branch;
        }

        if (nodeStack.empty()) {
            n = nullptr;
        } else {
            n = nodeStack.front();
            nodeStack.pop_front();
        }
    }

    // connect branches
    for (auto& kv : branchMap) {
        BranchRef destinationBranch = kv.second;
        n = destinationBranch->getNodes().front();

        std::vector<NodePortConnectionRef> connections = getInputConnectionsForNodeWithPortType(n, NodePortType::FBOImage);
        for (const NodePortConnectionRef& c : connections) {
            BranchRef sourceBranch = branchMap[c->getSourceNode()];
            destinationBranch->connectInputBranch(sourceBranch);
        }
    }

    return branchMap[node];
}

#pragma mark -

std::string Context::serialize() {
    std::map<NodeRef, std::string> nodeIdentifierMap;
    for (size_t idx = 0; idx < mNodes.size(); idx++) {
        const NodeRef& n = mNodes.at(idx);
        nodeIdentifierMap[n] = str(boost::format("%1%-%2%") % n->getName() % idx);
    }

    JsonTree rootObject = JsonTree::makeObject();
    for (const NodeRef& n : mNodes) {
        JsonTree nodeObject = JsonTree::makeObject();
        nodeObject.pushBack(JsonTree("identifier", nodeIdentifierMap[n]));
        // TODO - ??? need some way to get classname
        nodeObject.pushBack(JsonTree("type", "???"));

        JsonTree valuesObject = JsonTree::makeObject("values");
        for (const NodePortRef& port : n->getInputPorts()) {
            if (!n->hasValueForInputPortKey(port->getKey())) {
                continue;
            }

            switch (port->getType()) {
                case NodePortType::FBOImage:
                case NodePortType::Texture:
                    // NB - transient values, nothing to serialize
                    break;
                case NodePortType::Bool:
                    valuesObject.pushBack(JsonTree(port->getKey(), n->getValueForInputPortKey<bool>(port->getKey())));
                    break;
                case NodePortType::Float:
                    valuesObject.pushBack(JsonTree(port->getKey(), n->getValueForInputPortKey<float>(port->getKey())));
                    break;
                case NodePortType::Int:
                case NodePortType::Index:
                    valuesObject.pushBack(JsonTree(port->getKey(), n->getValueForInputPortKey<int>(port->getKey())));
                    break;
                case NodePortType::Vec2: {
                    JsonTree valueObject = JsonTree::makeObject(port->getKey());
                    vec2 val = n->getValueForInputPortKey<vec2>(port->getKey());
                    valueObject.pushBack(JsonTree("x", val.x));
                    valueObject.pushBack(JsonTree("y", val.y));
                    valuesObject.pushBack(valueObject);
                    break;
                }
                case NodePortType::Color: {
                    JsonTree valueObject = JsonTree::makeObject(port->getKey());
                    ColorAf val = n->getValueForInputPortKey<ColorAf>(port->getKey());
                    valueObject.pushBack(JsonTree("r", val.r));
                    valueObject.pushBack(JsonTree("g", val.g));
                    valueObject.pushBack(JsonTree("b", val.b));
                    valueObject.pushBack(JsonTree("a", val.a));
                    valuesObject.pushBack(valueObject);
                    break;
                }
                case NodePortType::FilePath: {
                    fs::path path = n->getValueForInputPortKey<fs::path>(port->getKey());
                    valuesObject.pushBack(JsonTree(port->getKey(), path.string()));
                    break;
                }
                default:
                    break;
            }
        }
        nodeObject.pushBack(valuesObject);

        JsonTree inputConnections = JsonTree::makeArray("inputConnections");
        for (const NodePortConnectionRef& c : getInputConnectionsForNodeWithPortType(n, NodePortType::FBOImage)) {
            JsonTree connectionObject = JsonTree::makeObject();
            connectionObject.pushBack(JsonTree("sourceNode", nodeIdentifierMap[c->getSourceNode()]));
            connectionObject.pushBack(JsonTree("sourcePortKey", c->getSourcePortKey()));
            connectionObject.pushBack(JsonTree("destinationPortKey", c->getDestinationPortKey()));
            inputConnections.pushBack(connectionObject);
        }
        nodeObject.pushBack(inputConnections);

        JsonTree outputConnections = JsonTree::makeArray("outputConnections");
        for (const NodePortConnectionRef& c : getOutputConnectionsForNodeWithPortType(n, NodePortType::FBOImage)) {
            JsonTree connectionObject = JsonTree::makeObject();
            connectionObject.pushBack(JsonTree("sourcePortKey", c->getSourcePortKey()));
            connectionObject.pushBack(JsonTree("destinationNode", nodeIdentifierMap[c->getDestinationNode()]));
            connectionObject.pushBack(JsonTree("destinationPortKey", c->getDestinationPortKey()));
            outputConnections.pushBack(connectionObject);
        }
        nodeObject.pushBack(outputConnections);

        rootObject.pushBack(nodeObject);
    }
    return rootObject.serialize();
}

bool Context::serialize(const fs::path& path) {
    std::ofstream outfile(path.string());
    if (!outfile.is_open()) {
        return false;
    }

    outfile << serialize();
    outfile.close();

    return true;
}

#pragma mark -

gl::Texture2dRef Context::evaluate(const NodeRef& node) {
    // rebuild render stack (flush cache) if the stack is empty or the node changes (cache key)
    if (mRenderStack.size() == 0 || node != mRenderNode) {
        // verify there are enough attachments
        auto result = std::max_element(mNodes.begin(), mNodes.end(), [](const NodeRef& n1, const NodeRef& n2) {
            return n1->getImageInputPortKeys().size() < n2->getImageInputPortKeys().size();
        });
        unsigned int count = mNodes.at(std::distance(mNodes.begin(), result))->getImageInputPortKeys().size();
        // NB - it appears a single node strand all with single inputs can be evaluated on a single attachment ¯\(°_o)/¯
        if (count != 1 && mAttachmentCount < count + 1) {
            cinder::app::console() << "ERROR - more attachments (color buffers) required" << std::endl;
        }

        mRenderNode = node;
        BranchRef root = branchForNode(mRenderNode);
        mRenderStack = renderStackForRootBranch(root);

#if defined(DEBUG)
        // ASCII visualization
        cinder::app::console() << std::string(3, '#') << std::endl;
        for (const BranchRef& b : mRenderStack) {
            cinder::app::console() << b->compactDescription() << std::endl;
        }
        cinder::app::console() << std::endl;
#endif
    }

    // render branches
    GLenum outAttachment = 0;

    gl::ScopedMatrices matricies;
    gl::ScopedViewport viewport(ivec2(0), mFBO->getSize());
    gl::ScopedFramebuffer fbo(mFBO);

    gl::setMatricesWindow(mFBO->getSize());
    gl::color(Color::white());

    std::vector<GLenum> availableAttachments;
    for (unsigned int idx = 0; idx < mAttachmentCount; idx++) {
        availableAttachments.push_back(GL_COLOR_ATTACHMENT0 + idx);
    }
    std::map<NodeRef, GLenum> attachmentsMap;

    for (const BranchRef& b : mRenderStack) {
        size_t attachmentIndex = 0;
        outAttachment = availableAttachments.at(attachmentIndex);
        GLenum inAttachment = 0;

        for (size_t nodeIdx = 0; nodeIdx < b->getNodes().size(); nodeIdx++) {
            // TODO - hoist draw buffer assignment
            NodeRef n = b->getNodes().at(nodeIdx);
            SourceNodeRef s = std::dynamic_pointer_cast<SourceNode>(n);
            if (s) {
                glDrawBuffer(outAttachment);

                FBOImageRef outputFBOImage = FBOImage::create(mFBO, outAttachment);
                s->render(outputFBOImage);

                inAttachment = outAttachment;
            } else {
                EffectorNodeRef e = std::dynamic_pointer_cast<EffectorNode>(n);
                if (e) {
                    std::vector<NodePortConnectionRef> connections = getInputConnectionsForNodeWithPortType(n, NodePortType::FBOImage);
                    if (connections.size() == 1) {
                        NodePortConnectionRef c = connections.at(0);
                        // grab attachment from map when appropriate
                        if (inAttachment == 0) {
                            inAttachment = attachmentsMap[c->getSourceNode()];
                            attachmentsMap.erase(c->getSourceNode());
                            availableAttachments.push_back(inAttachment);
                        }
                        FBOImageRef inputFBOImage = FBOImage::create(mFBO, inAttachment);
                        e->setValueForInputPortKey(inputFBOImage, c->getDestinationPortKey());

                        attachmentIndex = (attachmentIndex + 1) % availableAttachments.size();
                        outAttachment = availableAttachments.at(attachmentIndex);
                        glDrawBuffer(outAttachment);

                        FBOImageRef outputFBOImage = FBOImage::create(mFBO, outAttachment);
                        e->render(outputFBOImage);

                        inAttachment = outAttachment;
                    } else if (connections.size() > 1) {
                        for (const NodePortConnectionRef& c : connections) {
                            inAttachment = attachmentsMap[c->getSourceNode()];
                            attachmentsMap.erase(c->getSourceNode());
                            availableAttachments.push_back(inAttachment);

                            FBOImageRef inputFBOImage = FBOImage::create(mFBO, inAttachment);
                            e->setValueForInputPortKey(inputFBOImage, c->getDestinationPortKey());
                        }

                        glDrawBuffer(outAttachment);

                        FBOImageRef outputFBOImage = FBOImage::create(mFBO, outAttachment);
                        e->render(outputFBOImage);
                        
                        inAttachment = outAttachment;
                    }
                }
            }

            // stash output attachment and accompanying node when branch concludes
            if (nodeIdx == b->getNodes().size() - 1) {
                attachmentsMap[n] = outAttachment;
                availableAttachments.erase(std::find(availableAttachments.begin(), availableAttachments.end(), outAttachment));
            }
        }
    }

    return mFBO->getTexture2d(outAttachment);
}

#pragma mark - PRIVATE

std::deque<BranchRef> Context::renderStackForRootBranch(const BranchRef& branch) {
    std::deque<BranchRef> renderStack;
    std::deque<BranchRef> branchStack;

    BranchRef b = branch;
    while (b) {
        renderStack.push_front(b);

        std::vector<BranchConnectionRef> inputConnections = b->getInputConnections();
        if (inputConnections.empty()) {
            if (branchStack.empty()) {
                b = nullptr;
            } else {
                b = branchStack.front();
                branchStack.pop_front();
            }
        } else {
            // sort by cost DESC
            std::vector<BranchConnectionRef> sortedInputConnections = inputConnections;
            std::sort(sortedInputConnections.begin(), sortedInputConnections.end(), [](const BranchConnectionRef& c1, const BranchConnectionRef& c2) {
                return c1->getInputCost() > c2->getInputCost();
            });

            // follow cheapest path
            b = sortedInputConnections.back()->getSourceBranch();
            sortedInputConnections.pop_back();

            // push the others
            for (const BranchConnectionRef& c : sortedInputConnections) {
                branchStack.push_front(c->getSourceBranch());
            }
        }
    }

    return renderStack;
}

}}
