//
//  Pipeline.cpp
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 19 Apr 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#include "Pipeline.h"
#include "cinder/Utilities.h"

using namespace ci;

namespace Cinder { namespace Pipeline {

#define NUM_ATTACHMENTS 3

PipelineRef Pipeline::create() {
    return PipelineRef(new Pipeline())->shared_from_this();
}

Pipeline::Pipeline() {
}

Pipeline::~Pipeline() {
}

#pragma mark -

void Pipeline::setup(const Vec2i size) {
#if defined(DEBUG)
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    cinder::app::console() << "GL_RENDERER: " << renderer << std::endl;
    const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    cinder::app::console() << "GL_VENDOR: " << vendor << std::endl;
    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    cinder::app::console() << "GL_VERSION: " << version << std::endl;
    const char* shadingLanguageVersion = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    cinder::app::console() << "GL_SHADING_LANGUAGE_VERSION: " << shadingLanguageVersion << std::endl;

    std::string extensionsString = std::string(reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS)));
    std::vector<std::string> extensions = split(extensionsString, " ");
    extensions.erase(std::remove_if(extensions.begin(), extensions.end(), [](const std::string& s){ return s.empty(); }));
    cinder::app::console() << "GL_EXTENSIONS: " << std::endl;
    for (auto e : extensions) {
        cinder::app::console() << " " << e << std::endl;
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
//    if (NUM_ATTACHMENTS > GL_MAX_COLOR_ATTACHMENTS) {
//        cinder::app::console() << "ERROR - number of attachments '" << NUM_ATTACHMENTS << "' exceeds '" << texSize << "'" << std::endl;
//    }
//
//    float attachmentMemorySizeMB = size.x * size.y * 4 / 1024 / 1024;
//    float totalSizeMB = attachmentMemorySizeMB * NUM_ATTACHMENTS;
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
    format.enableColorBuffer(true, NUM_ATTACHMENTS);
    format.enableDepthBuffer(false);

    mFBO = gl::Fbo(size.x, size.y, format);
    mFBO.bindFramebuffer(); {
        GLenum buffers[NUM_ATTACHMENTS];
        for (unsigned int idx = 0; idx < NUM_ATTACHMENTS; idx++) {
            buffers[idx] = GL_COLOR_ATTACHMENT0 + idx;
        }
        glDrawBuffers(NUM_ATTACHMENTS, buffers);
        gl::setViewport(mFBO.getBounds());
        gl::clear();
    } mFBO.unbindFramebuffer();
}

gl::Texture& Pipeline::evaluate(const NodeRef& node) {
    BranchRef root = branchForNode(node);
    std::deque<BranchRef> renderStack = renderStackForRootBranch(root);

#if defined(DEBUG)
    // ASCII visualization
    cinder::app::console() << std::string(13, '#') << std::endl;
    for (BranchRef b : renderStack) {
        unsigned int spaceCount = b->getMaxInputCost() * 5 + MAX((int)b->getMaxInputCost() - 1, 0) * 3;
        cinder::app::console() << std::string(spaceCount, ' ');

        for (NodeRef n : b->getNodes()) {
            if (!std::dynamic_pointer_cast<SourceNode>(n)) {
                cinder::app::console() << " → ";
            }

            std::string name = n->getName();
            name.resize(3, ' ');
            cinder::app::console() << "[" << name << "]";
        }
        cinder::app::console() << std::endl;
    }
    cinder::app::console() << std::endl;
#endif

    // render branches
    unsigned int outAttachment = 0;

    Area viewport = gl::getViewport();
    gl::setViewport(mFBO.getBounds());
    mFBO.bindFramebuffer(); {
        gl::pushMatrices(); {
            gl::setMatricesWindow(mFBO.getSize(), false);

            // int instead of GLenum for Cinder FBO bindTexture/getTexture
            std::vector<int> availableAttachments;
            for (unsigned int idx = 0; idx < NUM_ATTACHMENTS; idx++) {
                availableAttachments.push_back(idx);
            }
            std::deque<std::tuple<int, NodeRef>> storedAttachments;

            for (BranchRef b : renderStack) {
                size_t attachmentIndex = 0;
                outAttachment = availableAttachments.at(attachmentIndex);
                int inAttachment = -1;
                for (size_t nodeIdx = 0; nodeIdx < b->getNodes().size(); nodeIdx++) {
                    NodeRef n = b->getNodes().at(nodeIdx);

                    SourceNodeRef s = std::dynamic_pointer_cast<SourceNode>(n);
                    if (s) {
                        s->render(mFBO, outAttachment);
                        inAttachment = outAttachment;
                    } else {
                        EffectorNodeRef e = std::dynamic_pointer_cast<EffectorNode>(n);
                        if (e) {
                            if (e->getInputNodes().size() == 1) {
                                if (availableAttachments.size() < 2) {
                                    // TODO - bad news
                                }

                                attachmentIndex = (attachmentIndex + 1) % availableAttachments.size();
                                outAttachment = availableAttachments.at(attachmentIndex);
                                e->render(mFBO, inAttachment, mFBO, outAttachment);
                                inAttachment = outAttachment;
                            } else if (e->getInputNodes().size() == 2) {
                                std::tuple<int, NodeRef> t = storedAttachments.front(); storedAttachments.pop_front();
                                inAttachment = std::get<0>(t);
                                std::tuple<int, NodeRef> t2 = storedAttachments.front(); storedAttachments.pop_front();
                                int inAltAttachment = std::get<0>(t2);

                                // ensure input ordering is correct
                                if (std::get<1>(t) != n->getInputNodes().at(0)) {
                                    std::swap(inAttachment, inAltAttachment);
                                }

                                availableAttachments.push_back(inAttachment);
                                availableAttachments.push_back(inAltAttachment);

                                e->render(mFBO, inAttachment, mFBO, inAltAttachment, mFBO, outAttachment);
                                inAttachment = outAttachment;
                            }
                        }
                    }

                    // stash output attachment and accompanying node when branch concludes
                    if (nodeIdx == b->getNodes().size() - 1) {
                        storedAttachments.push_front(std::make_tuple(outAttachment, n));
                        availableAttachments.erase(std::find(availableAttachments.begin(), availableAttachments.end(), outAttachment));
                    }
                }

            }
        } gl::popMatrices();
    } mFBO.unbindFramebuffer();
    gl::setViewport(viewport);

    return mFBO.getTexture(outAttachment);
}

#pragma mark - PRIVATE

BranchRef Pipeline::branchForNode(const NodeRef& node) {
    std::deque<NodeRef> nodes;
    BranchRef branch = Branch::create();

    NodeRef n = node;
    while (n) {
        nodes.push_front(n);

        if (std::dynamic_pointer_cast<SourceNode>(n)) {
            n = nullptr;
        } else {
            if (n->getInputNodes().size() == 1) {
                n = n->getInputNodes().at(0);
            } else if (n->getInputNodes().size() == 2) {
                BranchRef b = branchForNode(n->getInputNodes().at(0));
                branch->connectInputBranch(b);

                b = branchForNode(n->getInputNodes().at(1));
                branch->connectInputBranch(b);

                n = nullptr;
            }
        }
    }
    branch->setNodes(nodes);

    return branch;
}

std::deque<BranchRef> Pipeline::renderStackForRootBranch(const BranchRef& branch) {
    std::deque<BranchRef> renderStack;
    std::deque<BranchRef> branchStack;

    BranchRef b = branch;
    while (b) {
        renderStack.push_front(b);
        if (b->getInputBranches().empty()) {
            if (branchStack.empty()) {
                b = nullptr;
            } else {
                b = branchStack.front(); branchStack.pop_front();
            }
        } else {
            // take cheaper route, push more expensive
            if (std::get<1>(b->getInputBranches().at(0)) >= std::get<1>(b->getInputBranches().at(1))) {
                branchStack.push_front(std::get<0>(b->getInputBranches().at(0)));
                b = std::get<0>(b->getInputBranches().at(1));
            } else {
                branchStack.push_front(std::get<0>(b->getInputBranches().at(1)));
                b = std::get<0>(b->getInputBranches().at(0));
            }
        }
    }

    return renderStack;
}

}}
