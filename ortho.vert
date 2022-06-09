#version 410

// 頂点の位置
layout (location = 0) in vec4 position;

void main()
{
  gl_Position = position;
}
