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

// スレッド
#include <thread>
#include <atomic>

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
  const GLint nextLoc{ glGetUniformLocation(program, "past") };

  // キャプチャデバイスの準備
  cv::VideoCapture camera;
  if (!camera.open(0)) throw std::runtime_error("The capture device cannot be used.");

  // キャプチャデバイスから 1 フレーム取り込む
  cv::Mat image;
  if (!camera.read(image)) throw std::runtime_error("No frames has been grabbed.");

  // テクスチャの準備
  std::array<GLuint, 2> textures;
  glGenTextures(textures.size(), textures.data());
  for (const auto texture : textures)
  {
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.cols, image.rows, 0,
      GL_BGR, GL_UNSIGNED_BYTE, image.data);

    // テクスチャをサンプリングする方法の指定
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  }

  // フレームバッファオブジェクトのサイズ
  const GLsizei fboWidth{ image.cols };
  const GLsizei fboHeight{ image.rows };

  // フレームバッファオブジェクトのカラーバッファに使うテクスチャ
  GLuint color;
  glGenTextures(1, &color);
  glBindTexture(GL_TEXTURE_2D, color);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fboWidth, fboHeight, 0,
    GL_BGR, GL_UNSIGNED_BYTE, 0);

  // テクスチャをサンプリングする方法の指定
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  // フレームバッファオブジェクトの準備
  GLuint fbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  // フレームバッファオブジェクトにカラーバッファに使うテクスチャを組み込む
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color, 0);

  // レンダリングターゲット
  const GLenum bufs[] = { GL_COLOR_ATTACHMENT0 };

  // 標準のフレームバッファに戻す
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // 背景色の設定
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  // キャプチャスレッド起動
  bool run{ true };
  bool update{ false };
  std::atomic<bool> lock{ false };
  auto capture{ std::thread([&] {
    while (run) {
      update = camera.grab() && !lock.exchange(true) && camera.retrieve(image);
      lock.store(false);
    }
  }) };

  // ウィンドウが開いている間繰り返す
  while (window)
  {
    // フレームバッファオブジェクトへの描画
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // このフレームバッファオブジェクトにはデプスバッファが無い
    glDepthMask(GL_FALSE);

    // ビューポートをフレームバッファオブジェクトに合わせる
    glViewport(0, 0, fboWidth, fboHeight);

    // 画面クリア
    glClear(GL_COLOR_BUFFER_BIT);

    // プログラムオブジェクトの指定
    glUseProgram(program);

    // ウィンドウのアスペクト比
    const GLfloat aspect{ GLfloat(fboWidth) / GLfloat(fboHeight) };

    // uniform 変数に値を設定する
    glUniform1f(aspectLoc, aspect);
    glUniform1i(imageLoc, 0);
    glUniform1i(nextLoc, 1);

    // テクスチャユニット番号
    const int unit{ glfwGetKey(window.get(), GLFW_KEY_SPACE) == GLFW_RELEASE ? 1 : 0 };

    // カメラのフレームが更新されたら
    if (update && !lock.exchange(true))
    {
      // テクスチャユニットを指定する
      glActiveTexture(GL_TEXTURE0 + unit);
      glBindTexture(GL_TEXTURE_2D, textures[unit]);

      // テクスチャに転送する
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.cols, image.rows,
        GL_BGR, GL_UNSIGNED_BYTE, image.data);

      // 転送完了を通知する
      update = false;
      lock.store(false);
    }

    // 二つ目のテクスチャユニットを指定する
    glActiveTexture(GL_TEXTURE1 - unit);
    glBindTexture(GL_TEXTURE_2D, textures[1 - unit]);

    // 頂点配列オブジェクトの指定
    glBindVertexArray(vao);

    // レンダーターゲットの指定
    glDrawBuffers(std::size(bufs), bufs);

    // 図形の描画
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // 標準のフレームバッファへの転送
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    GLsizei w{ window.getWidth() };
    GLsizei h{ window.getHeight() };
    GLsizei x{ w / 2 };
    GLsizei y{ h / 2 };
    float t{ h * aspect };
    if (h * aspect > w) h = w / aspect; else w = t;
    x -= w / 2;
    y -= h / 2;
    glBlitFramebuffer(0, 0, fboWidth, fboHeight, x, y, x + w, y + h, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    // カラーバッファを入れ替えてイベントを取り出す
    window.swapBuffers();
  }

  // キャプチャスレッド停止
  run = false;
  capture.join();

  return EXIT_SUCCESS;
}
