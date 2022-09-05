// 補助プログラム
#include "GgApp.h"

// OpenCV
#include "opencv2/opencv.hpp"
#include "opencv2/aruco.hpp"
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
#  pragma comment(lib, "opencv_aruco" CV_VERSION_STR CV_EXT_STR)
#endif

// スレッド
#include <thread>
#include <atomic>

// PBO をマップしてマーカーを検出するとき
#define USE_PBO

// アプリケーション本体
int GgApp::main(int argc, const char* const* argv)
{
  // ウィンドウを作成する
  Window window;

  // 頂点配列オブジェクト
  GLuint vao;
  glGenVertexArrays(1, &vao);

  // プログラムオブジェクトの作成
  const GLuint program{ ggLoadShader("ortho.vert", "ortho.frag") };

  // uniform 変数の場所を調べる
  const GLint aspectLoc{ glGetUniformLocation(program, "aspect") };
  const GLint rotationLoc{ glGetUniformLocation(program, "rotation") };
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

  // フレームバッファの読み出しに使うピクセルバッファオブジェクトを作成する
  GLuint pixel;
  glGenBuffers(1, &pixel);

  // フレームバッファと同じサイズのメモリを確保する
  glBindBuffer(GL_PIXEL_PACK_BUFFER, pixel);
  glBufferData(GL_PIXEL_PACK_BUFFER, fboWidth * fboHeight * 3, nullptr, GL_DYNAMIC_COPY);
  glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

  // ピクセルバッファオブジェクトのデータのコピー先
  cv::Mat buffer{ cv::Size(fboWidth, fboHeight), CV_8UC3 };

  // ArUco Marker の辞書と検出パラメータ
  const cv::Ptr<cv::aruco::Dictionary> dictionary{ cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_1000) };
  const cv::Ptr<cv::aruco::DetectorParameters> parameters{ cv::aruco::DetectorParameters::create() };

  // 検出された ArUco Marker の ID と位置
  std::vector<int> ids;
  std::vector<std::vector<cv::Point2f>> corners, rejected;

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
    glUniformMatrix4fv(rotationLoc, 1, GL_FALSE, ggRotate(0.0f, 1.0f, 0.0f, 0.0f).get());
    glUniform1f(aspectLoc, aspect);
    glUniform1i(imageLoc, 0);
    glUniform1i(nextLoc, 1);

    // テクスチャユニット番号
    static int unit{ 0 };

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

      // テクスチャユニットを入れ替える
      unit = 1 - unit;
    }

    // 二つ目のテクスチャユニットを指定する
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, textures[unit]);

    // 頂点配列オブジェクトの指定
    glBindVertexArray(vao);

    // レンダーターゲットの指定
    glDrawBuffers(std::size(bufs), bufs);

    // 図形の描画
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 41 * 2 + 2, 31);

    glfwSetTime(0.0);
#if defined(USE_PBO)
    // フレームバッファオブジェクトからピクセルバッファオブジェクトにコピー
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pixel);
    glReadPixels(0, 0, fboWidth, fboHeight, GL_BGR, GL_UNSIGNED_BYTE, 0);

    // ピクセルバッファオブジェクトを CPU のメモリにマップ
    cv::Mat frame{ cv::Size(fboWidth, fboHeight), CV_8UC3,
      glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_WRITE) };

    // マーカーを検出する
    cv::aruco::detectMarkers(frame, dictionary, corners, ids, parameters, rejected);

    // ピクセルバッファオブジェクトを開放
    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    // マーカーが見つかったら
    if (ids.size() > 0)
    {
      // 検出結果を表示に描き込む
      cv::aruco::drawDetectedMarkers(frame, corners, ids);
      glBindTexture(GL_TEXTURE_2D, color);
      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixel);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, fboWidth, fboHeight,
        GL_BGR, GL_UNSIGNED_BYTE, 0);
      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
      glBindTexture(GL_TEXTURE_2D, 0);
    }
#else
    // ピクセルバッファオブジェクトから CPU のメモリにコピー
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixel);
    glGetBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, fboWidth * fboHeight * 3, buffer.data);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    // マーカーを検出する
    cv::aruco::detectMarkers(buffer, dictionary,
      markerCorners, markerIds, parameters, rejectedCandidates);

    // マーカーが見つかったら
    if (markerIds.size() > 0)
    {
      // 検出結果を表示に描き込む
      cv::aruco::drawDetectedMarkers(buffer, markerCorners, markerIds);
      glBindTexture(GL_TEXTURE_2D, color);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, fboWidth, fboHeight,
        GL_BGR, GL_UNSIGNED_BYTE, buffer.data);
      glBindTexture(GL_TEXTURE_2D, 0);
    }
#endif
    std::cerr << glfwGetTime() << '\n';

#ifdef _DEBUG
    for (auto& corner : corners)
    {
      for (auto& point : corner)
      {
        std::cout << "(" << point.x << ", " << point.y << ")";
      }
      std::cout << std::endl;
    }
#endif

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
    glBlitFramebuffer(0, 0, fboWidth, fboHeight, x, y + h, x + w, y, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    // カラーバッファを入れ替えてイベントを取り出す
    window.swapBuffers();
  }

  // キャプチャスレッド停止
  run = false;
  capture.join();

  // フレームバッファオブジェクトからの読み出し
  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
  cv::Mat result{ cv::Size2i(fboWidth, fboHeight), CV_8UC3 };
  glReadPixels(0, 0, fboWidth, fboHeight, GL_BGR, GL_UNSIGNED_BYTE, result.data);

  // 結果を保存
  cv::imwrite("result.jpg", result);

  return EXIT_SUCCESS;
}
