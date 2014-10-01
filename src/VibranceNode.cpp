//
//  VibranceNode.cpp
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 28 Sept 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#include "VibranceNode.h"

using namespace ci;
using namespace Cinder::Pipeline;

// NB - unlike Adobe's Vibrance, this does not treat skintones any differently
// https://github.com/v002/v002-Color-Controls/blob/master/v002.Vibrance.frag
// https://github.com/terrasque/sweetfxui/blob/master/SweetFX/SweetFX/Shaders/Vibrance.h
const std::string FragmentShaderVibrance = R"(
    #version 120
    uniform sampler2D image;
    uniform float amount;
    uniform float mixAmount;

    // http://en.wikipedia.org/wiki/Luma_(video)
    float lumaRec601(vec4 color) {
        return dot(color.rgb, vec3(0.299, 0.587, 0.114));
    }
    // http://en.wikipedia.org/wiki/Luminance_(colorimetry)
    float lumaRec709(vec4 color) {
        return dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    }

    // https://www.opengl.org/sdk/docs/man/html/sign.xhtml
    float sign(float value) {
        return (value < 0.0) ? -1.0 : ((value == 0.0) ? 0.0 : 1.0);
    }

    void main() {
        vec2 position = gl_TexCoord[0].xy;
        vec4 color = texture2D(image, position);

        float y = lumaRec709(color);
        float max = max(color.r, max(color.g, color.b));
        float min = min(color.r, min(color.g, color.b));
        float delta = max - min; // max - min ~= saturation

        vec4 result;
        result.rgb = mix(vec3(y), color.rgb, clamp(1.0 + (amount * (1.0 - (sign(amount) * delta))), 0.0, 1.0));
        result.a = 1.0;

        gl_FragColor = mix(color, result, mixAmount);
    }
)";

VibranceNode::VibranceNode() {
    std::vector<NodePortRef> inputPorts = {
        NodePort::create(NodeInputPortKeyImage, NodePortType::FBOImage),
        NodePort::create(VibranceNodeInputPortKeyAmount, NodePortType::Float, "Amount", 0.0f, -1.0f, 1.0f),
        NodePort::create(VibranceNodeInputPortKeyMixAmount, NodePortType::Float, "Mix", 1.0f, 0.0f, 1.0f),
    };
    setInputPorts(inputPorts);
    // NB - output port "image" of type NodePortType::FBOImage is already present

    setupShader(sVertexShaderPassThrough, FragmentShaderVibrance);
}

VibranceNode::~VibranceNode() {
}

void VibranceNode::render(const FBOImageRef& outputFBOImage) {
    FBOImageRef inputFBOImage = getValueForInputPortKey<FBOImageRef>(NodeInputPortKeyImage);
    float amount = getValueForInputPortKey<float>(VibranceNodeInputPortKeyAmount);
    float mixAmount = getValueForInputPortKey<float>(VibranceNodeInputPortKeyMixAmount);

    inputFBOImage->bindTexture(0); {
        mShader->bind(); {
            mShader->uniform(NodeInputPortKeyImage, 0);
            mShader->uniform(VibranceNodeInputPortKeyAmount, amount);
            mShader->uniform(VibranceNodeInputPortKeyMixAmount, mixAmount);
            gl::drawSolidRect(outputFBOImage->getFBO().getBounds());
        } mShader->unbind();
    } inputFBOImage->unbindTexture();
}
