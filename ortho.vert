#version 410

// 頂点の位置
layout (location = 0) in vec4 position;

// ウィンドウのアスペクト比
uniform float aspect;

// スクリーンまでの焦点距離
uniform float focal = 1.0;

// テクスチャ座標
out vec2 texcoord;

void main()
{
  // クリッピング空間いっぱいに描く
  gl_Position = position;

  // 大きさが 2π × 1 のスクリーン上の点の位置
  texcoord = (position.st * 0.5 + 0.5) * vec2(6.283185, 1.0);
}
