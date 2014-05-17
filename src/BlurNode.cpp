//
//  BlurNode.cpp
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 22 Apr 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#include "BlurNode.h"

using namespace Cinder::Pipeline;

// taken from https://github.com/paulhoux/Cinder-Samples/blob/master/BloomingNeon/assets/blur_frag.glsl
const std::string FragmentShaderBlur = R"(
    #version 120
    uniform sampler2D image;
    uniform vec2 sampleOffset;
    uniform float amount;

    void main() {
        vec2 position = gl_TexCoord[0].st;
        vec4 color = texture2D(image, position);

        vec4 sum = vec4(0.0);
        sum += texture2D(image, position + -10.0 * sampleOffset) * 0.009167927656011385;
        sum += texture2D(image, position +  -9.0 * sampleOffset) * 0.014053461291849008;
        sum += texture2D(image, position +  -8.0 * sampleOffset) * 0.020595286319257878;
        sum += texture2D(image, position +  -7.0 * sampleOffset) * 0.028855245532226279;
        sum += texture2D(image, position +  -6.0 * sampleOffset) * 0.038650411513543079;
        sum += texture2D(image, position +  -5.0 * sampleOffset) * 0.049494378859311142;
        sum += texture2D(image, position +  -4.0 * sampleOffset) * 0.060594058578763078;
        sum += texture2D(image, position +  -3.0 * sampleOffset) * 0.070921288047096992;
        sum += texture2D(image, position +  -2.0 * sampleOffset) * 0.079358891804948081;
        sum += texture2D(image, position +  -1.0 * sampleOffset) * 0.084895951965930902;
        sum += texture2D(image, position +   0.0 * sampleOffset) * 0.086826196862124602;
        sum += texture2D(image, position +  +1.0 * sampleOffset) * 0.084895951965930902;
        sum += texture2D(image, position +  +2.0 * sampleOffset) * 0.079358891804948081;
        sum += texture2D(image, position +  +3.0 * sampleOffset) * 0.070921288047096992;
        sum += texture2D(image, position +  +4.0 * sampleOffset) * 0.060594058578763078;
        sum += texture2D(image, position +  +5.0 * sampleOffset) * 0.049494378859311142;
        sum += texture2D(image, position +  +6.0 * sampleOffset) * 0.038650411513543079;
        sum += texture2D(image, position +  +7.0 * sampleOffset) * 0.028855245532226279;
        sum += texture2D(image, position +  +8.0 * sampleOffset) * 0.020595286319257878;
        sum += texture2D(image, position +  +9.0 * sampleOffset) * 0.014053461291849008;
        sum += texture2D(image, position + +10.0 * sampleOffset) * 0.009167927656011385;

        gl_FragColor = mix(color, sum, clamp(amount, 0.0, 1.0));
    }
)";

BlurNodeRef BlurNode::create() {
    return BlurNodeRef(new BlurNode());
}

BlurNode::BlurNode() {
    std::vector<NodePortRef> inputPorts = {
        NodePort::create("image", NodePortType::FBOImage),
        NodePort::create("sampleOffset", NodePortType::Vec2f),
    };
    setInputPorts(inputPorts);
//    std::vector<std::string> outputKeys = {"image"};
//    setOutputPortKeys(outputKeys);

    setupShader(sVertexShaderPassThrough, FragmentShaderBlur);
}

BlurNode::~BlurNode() {
}

void BlurNode::render(const FBOImageRef& outputFBOImage) {
    FBOImageRef inputFBOImage = getValueForInputPortKey<FBOImageRef>("image");
    Vec2f inputSampleOffset = getValueForInputPortKey<Vec2f>("sampleOffset");

    inputFBOImage->bindTexture(0); {
        mShader->bind(); {
            mShader->uniform("image", 0);
            mShader->uniform("sampleOffset", inputSampleOffset);
            mShader->uniform("amount", 1.0f);
            gl::drawSolidRect(outputFBOImage->getFBO().getBounds());
        } mShader->unbind();
    } inputFBOImage->unbindTexture();
}
