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

// Vibrance - saturate the desatured values, and desature the saturated values
// based on Vibrance.fs from VDMX5: http://vdmx.vidvox.net/blog/chroma-mask-audio-analysis-visualizations-and-more
// NB - unlike Adobe's Vibrance, this does not treat skintones any differently
const std::string FragmentShaderVibrance = R"(
    #version 120
    uniform sampler2D image;
    uniform float amount;
    uniform float mixAmount;

    // http://gamedev.stackexchange.com/a/59808
    vec3 rgb2hsv(vec3 c) {
        vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        float d = q.x - min(q.w, q.y);
        float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }

    vec3 hsv2rgb(vec3 c) {
        vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    void main() {
        vec2 position = gl_TexCoord[0].xy;
        vec4 color = texture2D(image, position);

        vec3 c = rgb2hsv(color.rgb);
        // plot:  http://po.st/oNWbxv
        if (amount >= 0.0) {
            c.y = amount * (sqrt(c.y) - c.y) + c.y;
        } else {
            c.y = amount * (sqrt(1.0 - c.y) - (1.0 - c.y)) + c.y;
        }

        vec4 result;
        result.rgb = hsv2rgb(c);
        result.a = color.a;

        gl_FragColor = mix(color, result, mixAmount);
    }
)";

VibranceNode::VibranceNode() {
    std::vector<NodePortRef> inputPorts = {
        NodePort::create(NodeInputPortKeyImage, NodePortType::FBOImage),
        NodePort::create(VibranceNodeInputPortKeyAmount, NodePortType::Float, "Amount", 0.0f, -1.0f, 0.6f),
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
