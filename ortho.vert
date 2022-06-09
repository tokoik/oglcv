#version 410

// 頂点の位置
layout (location = 0) in vec4 position;

// ウィンドウのアスペクト比
uniform float aspect;

void main()
{
  gl_Position = position * vec4(0.5 / aspect, 0.5, 0.5, 1.0);
}
