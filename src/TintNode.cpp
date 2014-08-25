//
//  TintNode.cpp
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 19 Jun 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#include "TintNode.h"

using namespace ci;
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
        NodePort::create(NodeInputPortKeyImage, NodePortType::FBOImage),
        // TODO - min and max
        NodePort::create(TintNodeInputPortKeyColor, NodePortType::Color, ColorAf(1.0, 1.0, 1.0, 1.0)),
        NodePort::create(TintNodeInputPortKeyAmount, NodePortType::Float, 1.0f, 0.0f, 1.0f),
    };
    setInputPorts(inputPorts);
    // NB - output port "image" of type NodePortType::FBOImage is already present

    setupShader(sVertexShaderPassThrough, FragmentShaderBlur);
}

TintNode::~TintNode() {
}

void TintNode::render(const FBOImageRef& outputFBOImage) {
    FBOImageRef inputFBOImage = getValueForInputPortKey<FBOImageRef>(NodeInputPortKeyImage);
    ColorAf tintColor = getValueForInputPortKey<ColorAf>(TintNodeInputPortKeyColor);
    float amount = getValueForInputPortKey<float>(TintNodeInputPortKeyAmount);

    inputFBOImage->bindTexture(0); {
        mShader->bind(); {
            mShader->uniform(NodeInputPortKeyImage, 0);
            mShader->uniform(TintNodeInputPortKeyColor, tintColor);
            mShader->uniform(TintNodeInputPortKeyAmount, amount);
            gl::drawSolidRect(outputFBOImage->getFBO().getBounds());
        } mShader->unbind();
    } inputFBOImage->unbindTexture();
}
