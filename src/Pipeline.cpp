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

    GLint dims[2];
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, dims);
    cinder::app::console() << "GL_MAX_VIEWPORT_DIMS: " << dims[0] << "x" << dims[1] << std::endl;
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
    cinder::app::console() << "-------------" << std::endl;
#endif

    gl::Fbo::Format format;
    format.enableColorBuffer(true, 3);
    format.enableDepthBuffer(false);
//    format.setWrap(GL_CLAMP, GL_CLAMP);
//    format.setMagFilter(GL_NEAREST);
//    format.setMinFilter(GL_LINEAR);

    mFBO = gl::Fbo(size.x, size.y, format);
    mFBO.bindFramebuffer(); {
        const GLenum buffers[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
        glDrawBuffers(3, buffers);
        gl::setViewport(mFBO.getBounds());
        gl::clear();
    } mFBO.unbindFramebuffer();

//    for (size_t idx = 0; idx < 3; ++idx) {
//        mFBO.getTexture(idx).setWrap(GL_CLAMP, GL_CLAMP);
//    }
}

gl::Texture& Pipeline::evaluate(const NodeRef& node) {
    // TODO - validation of node inputs

    // build branch list
    std::deque<std::deque<NodeRef>> branches;
    std::deque<NodeRef> branch;
    std::deque<NodeRef> stack;

    NodeRef n = node;
    while (n) {
        branch.push_front(n);

        SourceNodeRef s = std::dynamic_pointer_cast<SourceNode>(n);
        if (s) {
            branches.push_front(branch);
            branch = std::deque<NodeRef>();

            if (stack.empty()) {
                n = nullptr;
            } else {
                n = stack.front();
                stack.pop_front();
            }
        } else {
            if (n->getInputNodes().size() == 1) {
                n = n->getInputNodes()[0];
            } else if (n->getInputNodes().size() == 2) {
                branches.push_front(branch);
                branch = std::deque<NodeRef>();

                stack.push_front(n->getInputNodes()[0]);
                n = n->getInputNodes()[1];
            }
        }
    }

#if defined(DEBUG)
    // ASCII visualization
    cinder::app::console() << std::string(13, '#') << std::endl;
    unsigned int count = 0;
    for (std::deque<NodeRef> branch : branches) {
        SourceNodeRef s = std::dynamic_pointer_cast<SourceNode>(branch.at(0));
        if (!s) {
            unsigned int spaceCount = count * 5 + count * 3;
            cinder::app::console() << std::string(spaceCount, ' ');
        }

        for (NodeRef node : branch) {
            std::string name = node->getName();
            name.resize(3, ' ');
            cinder::app::console() << "[" << name << "] > ";
        }
        cinder::app::console() << std::endl;

        if (count == 0 || branch.size() > count) {
            count = branch.size();
        } else if (!s) {
            count += branch.size();
        }
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

            std::vector<int> attachments = {0, 1, 2};
            std::vector<int> storedAttachments;

            for (std::deque<NodeRef> branch : branches) {
                size_t attachmentIndex = 0;
                outAttachment = attachments.at(attachmentIndex);
                int inAttachment = -1;
                int inAltAttachment = -1;

                for (NodeRef n : branch) {
                    SourceNodeRef s = std::dynamic_pointer_cast<SourceNode>(n);
                    if (s) {
                        s->render(mFBO, outAttachment);
                        inAttachment = outAttachment;
                    } else {
                        EffectorNodeRef e = std::dynamic_pointer_cast<EffectorNode>(n);
                        if (!e) {
                            cinder::app::console() << "ERROR - unknown node type '" << n->getName() << "'" << std::endl;
                        } else {
                            if (e->getInputNodes().size() == 1) {
                                attachmentIndex = (attachmentIndex + 1) % attachments.size();
                                outAttachment = attachments.at(attachmentIndex);
                                e->render(mFBO, inAttachment, mFBO, outAttachment);
                                inAttachment = outAttachment;
                            } else if (e->getInputNodes().size() == 2) {
                                inAttachment = storedAttachments.at(0);
                                inAltAttachment = storedAttachments.at(1);

                                storedAttachments.clear();
                                attachments = {0, 1, 2};
                                attachmentIndex = outAttachment;

                                e->render(mFBO, inAttachment, mFBO, inAltAttachment, mFBO, outAttachment);
                                inAttachment = outAttachment;
                            }
                        }
                    }
                }

                storedAttachments.push_back(outAttachment);
                attachments.erase(std::find(attachments.begin(), attachments.end(), outAttachment));
            }
        } gl::popMatrices();
    } mFBO.unbindFramebuffer();
    gl::setViewport(viewport);

    return mFBO.getTexture(outAttachment);
}

}}
