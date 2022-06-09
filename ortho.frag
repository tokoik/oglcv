#version 410

// 画素の色
layout (location = 0) out vec4 color;

// サンプラ―
uniform sampler2D image;

// テクスチャ座標
in vec2 texcoord;

void main()
{
  // ITU-R BT.601
  float gray = dot(vec3(0.299, 0.587, 0.114), texture(image, texcoord).rgb);
  color = vec4(vec3(fwidth(gray)), 1.0);
}
