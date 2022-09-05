#version 410

// ウィンドウのアスペクト比
uniform float aspect;

// スクリーンまでの焦点距離
uniform float focal = 1.0;

// スクリーンを回転する変換行列
uniform mat4 rotation;

// 分割数
uniform int slices = 41, stacks = 31;

// テクスチャ座標
out vec3 texcoord;

void main()
{
  // 頂点番号から座標値を求める
  //
  //   gl_VertexID   x   y
  //   -------------------
  //             0  -1   1
  //             1  -1  -1
  //             2   1   1
  //             3   1  -1
  //
  int s = gl_VertexID & ~1;
  int t = (~gl_VertexID & 1) + gl_InstanceID << 1;
  vec2 position = vec2(s, t) / vec2(slices, stacks) - 1.0;

  // クリッピング空間いっぱいに描く
  gl_Position = vec4(position, 0.0, 1.0);

  // 大きさが aspect × 1.0 のスクリーン上の上下を反転した点の位置
  vec2 uv = position * vec2(aspect, -1.0);

  // 右手系で原点から z = -focal の位置にあるその点に向かうベクトル
  vec3 xyz = vec3(uv, -focal);

  // 回転の変換行列をかけて視線ベクトルを求める
  texcoord = normalize(mat3(rotation) * xyz);
}
