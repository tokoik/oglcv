#version 410

// 頂点の位置
layout (location = 0) in vec4 position;

// ウィンドウのアスペクト比
uniform float aspect;

// スクリーンまでの焦点距離
uniform float focal = 1.0;

// スクリーンを回転する変換行列
uniform mat4 rotation;

// テクスチャ座標
out vec3 texcoord;

void main()
{
  // クリッピング空間いっぱいに描く
  gl_Position = position;

  // 大きさが aspect × 1.0 のスクリーン上の点の位置
  vec2 uv = position.st * vec2(aspect, 1.0);

  // 右手系で原点から z = -focal の位置にあるその点に向かうベクトル
  vec3 xyz = vec3(uv, -focal);

  // 回転の変換行列をかけて視線ベクトルを求める
  texcoord = normalize(mat3(rotation) * xyz);
}
