//
//  BlendNode.cpp
//  Cinder-Pipeline
//
//  Created by Jean-Pierre Mouilleseaux on 24 Apr 2014.
//  Copyright 2014 Chorded Constructions. All rights reserved.
//

#include "BlendNode.h"

using namespace Cinder::Pipeline;

// http://en.wikipedia.org/wiki/Alpha_compositing
// http://cairographics.org/operators/
// http://www.svgopen.org/2005/papers/abstractsvgopen/
const std::string FragmentShaderBlend = R"(
    #version 120
    uniform sampler2D image;
    uniform sampler2D blendImage;
    uniform int blendOperation;

    #define MODE_SUBTRACT 0
    #define MODE_OVER 1
    #define MODE_MULTIPLY 2

    void main() {
        vec2 position = gl_TexCoord[0].st;
        vec4 baseColor = texture2D(image, position);
        vec4 blendColor = texture2D(blendImage, position);

        if (blendOperation == MODE_SUBTRACT) {
            baseColor.rgb *= baseColor.a;
            blendColor.rgb *= blendColor.a;
            gl_FragColor = max(baseColor - blendColor, vec4(0.0));
        } else if (blendOperation == MODE_OVER) {
            baseColor.rgb *= baseColor.a;
            blendColor.rgb *= blendColor.a;
            gl_FragColor.rgb = blendColor.rgb + baseColor.rgb * (1.0 - blendColor.a);
            gl_FragColor.a = blendColor.a + baseColor.a * (1.0 - blendColor.a);
        } else if (blendOperation == MODE_MULTIPLY) {
            baseColor.rgb *= baseColor.a;
            blendColor.rgb *= blendColor.a;
            gl_FragColor.rgb = baseColor.rgb * blendColor.rgb;
            gl_FragColor.a = 1.0;
        }
    }
)";

BlendNode::BlendNode() {
    std::vector<NodePortRef> inputPorts = {
        NodePort::create(NodeInputPortKeyImage, NodePortType::FBOImage),
        NodePort::create(BlendNodeInputPortKeyBlendImage, NodePortType::FBOImage),
        // TODO - set possible values
        NodePort::create(BlendNodeInputPortKeyOperation, NodePortType::Int, static_cast<int>(BlendOperation::Over)),
    };
    setInputPorts(inputPorts);

    setupShader(sVertexShaderPassThrough, FragmentShaderBlend);
}

BlendNode::~BlendNode() {
}

void BlendNode::render(const FBOImageRef& outputFBOImage) {
    FBOImageRef inputFBOImage = getValueForInputPortKey<FBOImageRef>(NodeInputPortKeyImage);
    FBOImageRef inputFBOImage2 = getValueForInputPortKey<FBOImageRef>(BlendNodeInputPortKeyBlendImage);
    int blendOperation = getValueForInputPortKey<int>(BlendNodeInputPortKeyOperation);

    inputFBOImage->bindTexture(0); {
        inputFBOImage2->bindTexture(1); {
            mShader->bind(); {
                mShader->uniform(NodeInputPortKeyImage, 0);
                mShader->uniform(BlendNodeInputPortKeyBlendImage, 1);
                mShader->uniform(BlendNodeInputPortKeyOperation, blendOperation);
                gl::drawSolidRect(outputFBOImage->getFBO().getBounds());
            } mShader->unbind();
        } inputFBOImage2->unbindTexture();
    } inputFBOImage->unbindTexture();
}
