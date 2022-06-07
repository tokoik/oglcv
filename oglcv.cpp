// 補助プログラム
#include "GgApp.h"

// アプリケーション本体
int GgApp::main(int argc, const char* const* argv)
{
  // ウィンドウを作成する
  Window window;

  // ウィンドウが開いている間繰り返す
  while (window)
  {
    //
    // ここで図形を描く
    //

    // カラーバッファを入れ替えてイベントを取り出す
    window.swapBuffers();
  }

  return EXIT_SUCCESS;
}
