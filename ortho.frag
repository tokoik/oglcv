#version 410

// 画素の色
layout (location = 0) out vec4 color;

// 魚眼レンズの画角
uniform vec2 fov = vec2(3.839724);

// テクスチャ座標に対するスケール
uniform vec2 scale = vec2(0.5, -0.5);

// テクスチャ座標の中心位置
uniform vec2 center = vec2(0.5);

// スクリーンを回転する変換行列
uniform mat4 rotation;

// サンプラ―
uniform sampler2D image;
uniform sampler2D past;

// テクスチャ座標
in vec2 texcoord;

void main()
{
  // テクスチャ座標から視線ベクトルを求める
  vec3 direction = mat3(rotation) * vec3(
    cos(texcoord.x),        // d = 1
    sin(texcoord.x),        // d = 1
    1.0 - texcoord.y * 2.0  // h = -1
  );

  // テクスチャ座標を求める
  vec2 st = normalize(direction.xy) * acos(-direction.z) * 2.0 / fov;

  // テクスチャのアスペクト比の逆数
  vec2 size = vec2(textureSize(image, 0));

  // テクスチャのアスペクト比 size / size.y に合わせてサンプリングする
  color = texture(image, st * scale * size.y / size + center);
}
