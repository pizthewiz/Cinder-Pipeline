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

    void main() {
        vec2 position = gl_TexCoord[0].st;
        vec4 baseColor = texture2D(image, position);
        vec4 blendColor = texture2D(blendImage, position);

        if (blendOperation == MODE_SUBTRACT) {
            blendColor.rgb *= blendColor.a;
            gl_FragColor = max(baseColor - blendColor, vec4(0.0));
        } else if (blendOperation == MODE_OVER) {
            baseColor.rgb *= baseColor.a;
            blendColor.rgb *= blendColor.a;
            vec4 color = vec4(0.0);
            color.rgb = blendColor.rgb + baseColor.rgb * (1.0 - blendColor.a);
            color.a = blendColor.a + baseColor.a * (1.0 - blendColor.a);
            gl_FragColor = color;
        }
    }
)";

BlendNode::BlendNode() {
    std::vector<NodePortRef> inputPorts = {
        NodePort::create("image", NodePortType::FBOImage),
        NodePort::create("blendImage", NodePortType::FBOImage),
        // TODO - set possible values
        NodePort::create("blendOperation", NodePortType::Int, BlendOperation::Over),
    };
    setInputPorts(inputPorts);

    setupShader(sVertexShaderPassThrough, FragmentShaderBlend);
}

BlendNode::~BlendNode() {
}

void BlendNode::render(const FBOImageRef& outputFBOImage) {
    FBOImageRef inputFBOImage = getValueForInputPortKey<FBOImageRef>("image");
    FBOImageRef inputFBOImage2 = getValueForInputPortKey<FBOImageRef>("blendImage");
    int blendOperation = getValueForInputPortKey<int>("blendOperation");

    inputFBOImage->bindTexture(0); {
        inputFBOImage2->bindTexture(1); {
            mShader->bind(); {
                mShader->uniform("image", 0);
                mShader->uniform("blendImage", 1);
                mShader->uniform("blendOperation", blendOperation);
                gl::drawSolidRect(outputFBOImage->getFBO().getBounds());
            } mShader->unbind();
        } inputFBOImage2->unbindTexture();
    } inputFBOImage->unbindTexture();
}
