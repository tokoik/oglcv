#version 410

// 画素の色
layout (location = 0) out vec4 color;

// サンプラ―
uniform sampler2D image;
uniform sampler2D past;

// テクスチャ座標
in vec2 texcoord;

void main()
{
  color = vec4(abs(texture(image, texcoord) - texture(past, texcoord)).rgb, 1.0);
}
