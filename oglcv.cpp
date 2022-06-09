// 補助プログラム
#include "GgApp.h"

// アプリケーション本体
int GgApp::main(int argc, const char* const* argv)
{
  // ウィンドウを作成する
  Window window;

  // 頂点配列オブジェクト
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // 頂点バッファオブジェクト
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  // 頂点属性
  static GLfloat position[][2]
  {
    { -1.0f,  1.0f },
    { -1.0f, -1.0f },
    {  1.0f,  1.0f },
    {  1.0f, -1.0f }
  };

  // 頂点属性の転送
  glBufferData(GL_ARRAY_BUFFER, sizeof position, position, GL_STATIC_DRAW);

  // attribute 変数の設定
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  // 頂点配列オブジェクト完成
  glBindVertexArray(0);

  // 背景色の設定
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  // ウィンドウが開いている間繰り返す
  while (window)
  {
    // 画面クリア
    glClear(GL_COLOR_BUFFER_BIT);

    //
    // ここで図形を描く
    //

    // カラーバッファを入れ替えてイベントを取り出す
    window.swapBuffers();
  }

  return EXIT_SUCCESS;
}
