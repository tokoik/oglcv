#version 410

// 画素の色
layout (location = 0) out vec4 color;

// サンプラ―
uniform sampler2D image;

// テクスチャ座標
in vec2 texcoord;

void main()
{
  color = texture(image, texcoord);
}
