#version 410

// 頂点の位置
layout (location = 0) in vec4 position;

// ウィンドウのアスペクト比
uniform float aspect;

// テクスチャのサンプラ
uniform sampler2D image;

// テクスチャ座標
out vec2 texcoord;

void main()
{
  vec2 size = vec2(textureSize(image, 0));
  gl_Position = position * vec4(size.x / size.y / aspect, 1.0, 1.0, 1.0);
  texcoord = position.xy * vec2(0.5, -0.5) + 0.5;
}
