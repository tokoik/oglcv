#version 410

// 画素の色
layout (location = 0) out vec4 color;

// 魚眼レンズの画角
uniform vec2 fov = vec2(3.839724);

// テクスチャ座標に対するスケール
uniform vec2 scale = vec2(0.5, -0.5);

// テクスチャ座標の中心位置
uniform vec2 center = vec2(0.5);

// サンプラ―
uniform sampler2D image;
uniform sampler2D past;

// テクスチャ座標
in vec3 texcoord;

void main()
{
  // 補間された視線ベクトルを正規化する
  vec3 direction = normalize(texcoord);

  // テクスチャ座標を求める
  vec2 st = normalize(direction.xy) * acos(-direction.z) * 2.0 / fov;

  // テクスチャのアスペクト比の逆数
  vec2 size = vec2(textureSize(image, 0));

  // テクスチャのアスペクト比 size / size.y に合わせてサンプリングする
  color = texture(image, st * scale * size.y / size + center);
}
