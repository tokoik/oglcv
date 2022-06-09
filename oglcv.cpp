// 補助プログラム
#include "GgApp.h"

// アプリケーション本体
int GgApp::main(int argc, const char* const* argv)
{
  // ウィンドウを作成する
  Window window;

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
