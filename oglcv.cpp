// 補助プログラム
#include "GgApp.h"

// OpenCV
#include "opencv2/opencv.hpp"
#if defined(_MSC_VER)
#  define CV_VERSION_STR \
     CVAUX_STR(CV_MAJOR_VERSION) CVAUX_STR(CV_MINOR_VERSION) CVAUX_STR(CV_SUBMINOR_VERSION)
#  if defined(_DEBUG)
#    define CV_EXT_STR "d.lib"
#  else
#    define CV_EXT_STR ".lib"
#  endif
#  pragma comment(lib, "opencv_core" CV_VERSION_STR CV_EXT_STR)
#  pragma comment(lib, "opencv_imgcodecs" CV_VERSION_STR CV_EXT_STR)
#  pragma comment(lib, "opencv_videoio" CV_VERSION_STR CV_EXT_STR)
#endif

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

  // プログラムオブジェクトの作成
  const GLuint program{ ggLoadShader("ortho.vert", "ortho.frag") };

  // uniform 変数の場所を調べる
  const GLint aspectLoc{ glGetUniformLocation(program, "aspect") };
  const GLint imageLoc{ glGetUniformLocation(program, "image") };

  // キャプチャデバイスの準備
  cv::VideoCapture camera;
  if (!camera.open(0)) throw std::runtime_error("The capture device cannot be used.");

  // キャプチャデバイスから 1 フレーム取り込む
  cv::Mat image;
  if (!camera.read(image)) throw std::runtime_error("No frames has been grabbed.");

  // テクスチャの準備
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.cols, image.rows, 0,
    GL_BGR, GL_UNSIGNED_BYTE, image.data);

  // テクスチャをサンプリングする方法の指定
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  // 背景色の設定
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  // ウィンドウが開いている間繰り返す
  while (window)
  {
    // 画面クリア
    glClear(GL_COLOR_BUFFER_BIT);

    // プログラムオブジェクトの指定
    glUseProgram(program);

    // ウィンドウのアスペクト比
    const GLfloat aspect{ GLfloat(window.getWidth()) / GLfloat(window.getHeight()) };

    // uniform 変数に値を設定する
    glUniform1f(aspectLoc, aspect);
    glUniform1i(imageLoc, 0);

    // カメラのフレームが取り込めたら
    if (camera.read(image))
    {
      // テクスチャユニットを指定する
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture);

      // テクスチャに転送する
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.cols, image.rows,
        GL_BGR, GL_UNSIGNED_BYTE, image.data);
    }

    // 頂点配列オブジェクトの指定
    glBindVertexArray(vao);

    // 図形の描画
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // カラーバッファを入れ替えてイベントを取り出す
    window.swapBuffers();
  }

  return EXIT_SUCCESS;
}
