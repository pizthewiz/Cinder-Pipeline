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
    uniform float mix;

    // http://en.wikipedia.org/wiki/Luma_(video)
    float lumaRec601(vec4 color) {
        return dot(color.rgb, vec3(0.299, 0.587, 0.114));
    }
    // http://en.wikipedia.org/wiki/Luminance_(colorimetry)
    float lumaRec709(vec4 color) {
        return dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    }

    void main() {
        vec2 position = gl_TexCoord[0].xy;
        vec4 color = texture2D(image, position);
        float y = lumaRec601(color);
        vec4 result = tintColor * vec4(y, y, y, 1.0);
        gl_FragColor = mix(color, result, mix);
    }
)";

TintNode::TintNode() {
    std::vector<NodePortRef> inputPorts = {
        NodePort::create(NodeInputPortKeyImage, NodePortType::FBOImage),
        NodePort::create(TintNodeInputPortKeyColor, NodePortType::Color, "Color", ColorAf(1.0, 1.0, 1.0, 1.0)),
        NodePort::create(TintNodeInputPortKeyMix, NodePortType::Float, "Mix", 1.0f, 0.0f, 1.0f),
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
    float mix = getValueForInputPortKey<float>(TintNodeInputPortKeyMix);

    inputFBOImage->bindTexture(0); {
        mShader->bind(); {
            mShader->uniform(NodeInputPortKeyImage, 0);
            mShader->uniform(TintNodeInputPortKeyColor, tintColor);
            mShader->uniform(TintNodeInputPortKeyMix, mix);
            gl::drawSolidRect(outputFBOImage->getFBO().getBounds());
        } mShader->unbind();
    } inputFBOImage->unbindTexture();
}
