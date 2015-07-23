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
#include "cinder/Log.h"

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

    // dump capabilities
//    CI_LOG_V(std::string(13, '-'));
//    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
//    CI_LOG_V(str(boost::format("GL_RENDERER: %1%") % renderer));
//    const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
//    CI_LOG_V(str(boost::format("GL_VENDOR: %1%") % vendor));
//    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
//    CI_LOG_V(str(boost::format("GL_VERSION: %1%") % version));
//    const char* shadingLanguageVersion = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
//    CI_LOG_V(str(boost::format("GL_SHADING_LANGUAGE_VERSION: %1%") % shadingLanguageVersion));
//
//    CI_LOG_V("GL_EXTENSIONS: ");
//    GLint extensionCount = 0;
//    glGetIntegerv(GL_NUM_EXTENSIONS, &extensionCount);
//    for (GLint idx = 0; idx < extensionCount; idx++) {
//        std::string extension(reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, idx)));
//        CI_LOG_V(str(boost::format("  %1%") % extension));
//    }
//
//    GLint texSize;
//    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);
//    CI_LOG_V(str(boost::format("GL_MAX_TEXTURE_SIZE: %1%") % texSize));
//    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &texSize);
//    CI_LOG_V(str(boost::format("GL_MAX_3D_TEXTURE_SIZE: %1%") % texSize));
//    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texSize);
//    CI_LOG_V(str(boost::format("GL_MAX_TEXTURE_IMAGE_UNITS: %1%") % texSize));
//    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &texSize);
//    CI_LOG_V(str(boost::format("GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS: %1%") % texSize));
//    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &texSize);
//    CI_LOG_V(str(boost::format("GL_MAX_COLOR_ATTACHMENTS: %1%") % texSize));
//    CI_LOG_V(std::string(13, '-'));

    // checks
    GLint texSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);
    CI_ASSERT_MSG(texSize >= size.x, str(boost::format("width %1% exceeds maximum texture size %2%") % size.x % texSize).c_str());
    CI_ASSERT_MSG(texSize >= size.y, str(boost::format("height %1% exceeds maximum texture size %2%") % size.y % texSize).c_str());
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &texSize);
    CI_ASSERT_MSG(GL_MAX_COLOR_ATTACHMENTS >= attachmentCount, str(boost::format("attachment count %1% exceeds maximum %2%") % attachmentCount % texSize).c_str());

//    float attachmentMemorySizeMB = size.x * size.y * 4 / 1024 / 1024;
//    float totalSizeMB = attachmentMemorySizeMB * attachmentCount;
//
//    if (gl::isExtensionAvailable("GL_NV_texture_barrier")) {
//        if (texSize >= 2 * size.x && texSize >= size.y) {
//            // TODO: double-wide
//        } else if (texSize >= 2 * size.y && texSize >= size.x) {
//            // TODO: double-tall
//        } else {
//            // TODO: tile
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

#pragma mark - SERIALIZATION

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

#pragma mark - EVALUATION

std::deque<std::deque<NodeRef>> Context::renderStackForRenderNode(const NodeRef& node) {
    // dependency solver via three-pass stratagem:
    //  [1] find valid connections and leaf nodes required for render node evaluation
    //  [2] calculate max distance from leaf nodes to render node
    //  [3] create a render stack from the bottom up, choose cheap first

    // [1] generate list of valid connections and leaf nodes
    std::vector<NodePortConnectionRef> connections;
    std::vector<NodeRef> leafNodes;
    std::function<void (NodeRef)> walkUp = [&](NodeRef n) {
        auto inputConections = getInputConnectionsForNodeWithPortType(n, NodePortType::FBOImage);
        if (inputConections.empty()) {
            // avoid duplicates
            if (std::find(std::begin(leafNodes), std::end(leafNodes), n) ==  std::end(leafNodes)) {
                leafNodes.push_back(n);
            }
            return;
        }

        for (auto connection : inputConections) {
            assert(std::find(std::begin(connections), std::end(connections), connection) == std::end(connections));
            connections.push_back(connection);
            walkUp(connection->getSourceNode());
        }
    };
    walkUp(node);

    // [2] calculate max distances from leaf node to render node
    std::map<NodePortConnectionRef, int> downEdgeCostMap;
    std::function<void (NodeRef)> walkDown = [&](NodeRef n) {
        int cost = 0;
        for (auto connection : getInputConnectionsForNodeWithPortType(n, NodePortType::FBOImage)) {
            if (downEdgeCostMap.count(connection) != 0 && downEdgeCostMap[connection] > cost) {
                cost = downEdgeCostMap[connection];
            }
        }
        cost++;

        // TODO: use some sort of std::filter
        for (auto connection : getOutputConnectionsForNodeWithPortType(n, NodePortType::FBOImage)) {
            if (std::find(std::begin(connections), std::end(connections), connection) != std::end(connections)) {
                downEdgeCostMap[connection] = cost;
                walkDown(connection->getDestinationNode());
            }
        }
    };
    for (auto n : leafNodes) {
        walkDown(n);
    }

    // [3] create render stack
    std::deque<std::deque<NodeRef>> renderStack = {{}};
    std::function<void (NodeRef)> upStack = [&](NodeRef n) {
        renderStack.front().push_front(n);

        std::vector<NodePortConnectionRef> sortedInputConnections = getInputConnectionsForNodeWithPortType(n, NodePortType::FBOImage);
        std::sort(sortedInputConnections.begin(), sortedInputConnections.end(), [&](const NodePortConnectionRef& c1, const NodePortConnectionRef& c2) {
            return downEdgeCostMap[c1] < downEdgeCostMap[c2];
        });

        if (!sortedInputConnections.empty()) {
            auto connection = sortedInputConnections.front();
            sortedInputConnections.erase(sortedInputConnections.begin());
            upStack(connection->getSourceNode());
            
            for (auto connection : sortedInputConnections) {
                renderStack.push_front(std::deque<NodeRef> ());
                upStack(connection->getSourceNode());
            }
        }
    };
    upStack(node);
    
    return renderStack;
}


gl::Texture2dRef Context::evaluate(const NodeRef& node) {
    // rebuild render stack (flush cache) if the stack is empty or the node changes (cache key)
    if (mRenderStack.size() == 0 || node != mRenderNode) {
        // verify there are enough attachments
        auto result = std::max_element(mNodes.begin(), mNodes.end(), [](const NodeRef& n1, const NodeRef& n2) {
            return n1->getImageInputPortKeys().size() < n2->getImageInputPortKeys().size();
        });
        unsigned int count = mNodes.at(std::distance(mNodes.begin(), result))->getImageInputPortKeys().size();
        // NB: it appears a single node strand with single inputs can technically be evaluated on a single attachment
        if (mAttachmentCount < count + 1) {
            CI_LOG_E("more attachments (color buffers) required");
            return nullptr;
        }

        mRenderStack = renderStackForRenderNode(node);
        mRenderNode = node;

        // dump render stack contents
        CI_LOG_V(std::string(3, '#'));
        for (auto b : mRenderStack) {
            for (auto n : b) {
                std::string name = n->getName();
                name.resize(3, ' ');
                CI_LOG_V(str(boost::format("[%1%]") % name));
            }
            CI_LOG_V("");
        }
        CI_LOG_V(std::string(3, '#'));
    }

    // render branches
    gl::ScopedMatrices matricies;
    gl::ScopedViewport viewport(ivec2(0), mFBO->getSize());
    gl::ScopedFramebuffer fbo(mFBO);

    gl::setMatricesWindow(mFBO->getSize());
    gl::color(Color::white());

    std::deque<GLenum> attachmentsQueue;
    for (unsigned int idx = 0; idx < mAttachmentCount; idx++) {
        attachmentsQueue.push_back(GL_COLOR_ATTACHMENT0 + idx);
    }
    std::map<NodeRef, GLenum> attachmentsMap;

    for (const std::deque<NodeRef>& b : mRenderStack) {
        for (const NodeRef& n : b) {
            assert(attachmentsQueue.size() != 0);
            GLenum outAttachment = attachmentsQueue.front();
            attachmentsQueue.pop_front();
            glDrawBuffer(outAttachment);

            std::vector<NodePortConnectionRef> connections = getInputConnectionsForNodeWithPortType(n, NodePortType::FBOImage);
            for (const NodePortConnectionRef& c : connections) {
                assert(attachmentsMap.count(c->getSourceNode()) != 0);
                GLenum inAttachment = attachmentsMap[c->getSourceNode()];
                // mark attachment for recycle
                attachmentsMap.erase(c->getSourceNode());
                attachmentsQueue.push_back(inAttachment);

                FBOImageRef inputFBOImage = FBOImage::create(mFBO, inAttachment);
                n->setValueForInputPortKey(inputFBOImage, c->getDestinationPortKey());
            }

            FBOImageRef outputFBOImage = FBOImage::create(mFBO, outAttachment);
            n->render(outputFBOImage);

            attachmentsMap[n] = outAttachment;
        }
    }

    return mFBO->getTexture2d(attachmentsMap[mRenderNode]);
}

}}
