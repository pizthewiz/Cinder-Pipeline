//
//  TintNode.cpp
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 19 Jun 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#include "TintNode.h"

using namespace Cinder::Pipeline;

const std::string FragmentShaderBlur = R"(
    #version 120
    uniform sampler2D image;
    uniform vec4 tintColor;
    uniform float amount;

    void main() {
        vec2 position = gl_TexCoord[0].xy;
        vec4 color = texture2D(image, position);
        // NTSC conversion weights
        float grey = dot(color.rgb, vec3(0.299, 0.587, 0.114));
        vec4 result = tintColor * vec4(grey, grey, grey, 1.0);
        gl_FragColor = mix(color, result, clamp(amount, 0.0, 1.0));
    }
)";

TintNode::TintNode() {
    std::vector<NodePortRef> inputPorts = {
        NodePort::create("image", NodePortType::FBOImage),
        // TODO - default of Vec4f(1.0, 1.0, 1.0, 1.0)
        NodePort::create("tintColor", NodePortType::Vec4f),
        // TODO - default of 1.0f, max 1.0f, min 0.0f
        NodePort::create("amount", NodePortType::Float),
    };
    setInputPorts(inputPorts);
    // NB - output port "image" of type NodePortType::FBOImage is already present

    setupShader(sVertexShaderPassThrough, FragmentShaderBlur);
}

TintNode::~TintNode() {
}

void TintNode::render(const FBOImageRef& outputFBOImage) {
    FBOImageRef inputFBOImage = getValueForInputPortKey<FBOImageRef>("image");
    Vec4f tintColor = getValueForInputPortKey<Vec4f>("tintColor");
    float amount = getValueForInputPortKey<float>("amount");

    inputFBOImage->bindTexture(0); {
        mShader->bind(); {
            mShader->uniform("image", 0);
            mShader->uniform("tintColor", tintColor);
            mShader->uniform("amount", amount);
            gl::drawSolidRect(outputFBOImage->getFBO().getBounds());
        } mShader->unbind();
    } inputFBOImage->unbindTexture();
}
