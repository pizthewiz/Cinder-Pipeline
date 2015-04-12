//
//  EffectorNode.cpp
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 19 May 2014.
//  Copyright 2014-2015 Chorded Constructions. All rights reserved.
//

#include "EffectorNode.h"

namespace Cinder { namespace Pipeline {

using namespace ci;

const std::string EffectorNode::sVertexShaderPassThrough = R"(
    #version 150
    uniform mat4 ciModelViewProjection;

    in vec4 ciPosition;
    in vec2	ciTexCoord0;

    out vec2 vTexCoord0;

    void main() {
        vTexCoord0 = ciTexCoord0;
        gl_Position = ciModelViewProjection * ciPosition;
    }
)";

const std::string EffectorNode::sFragmentShaderPassThrough = R"(
    #version 150
    uniform sampler2D image;

    in vec2 vTexCoord0;

    out vec4 oFragColor;

    void main() {
        oFragColor = texture(image, vTexCoord0);
    }
)";

void EffectorNode::setupShader(const DataSourceRef& vertexShader, const DataSourceRef& fragmentShader) {
    std::string vert = vertexShader ? loadString(vertexShader) : sVertexShaderPassThrough;
    std::string frag = loadString(fragmentShader);
    setupShader(vert, frag);
}

void EffectorNode::setupShader(const std::string& vertexShader, const std::string& fragmentShader) {
    std::string vert = !vertexShader.empty() ? vertexShader : sVertexShaderPassThrough;
    auto format = gl::GlslProg::Format().vertex(vert).fragment(fragmentShader);
    try {
        mShader = gl::GlslProg::create(format);
    } catch (gl::GlslProgExc e) {
        throw e;
    }
}

}}
