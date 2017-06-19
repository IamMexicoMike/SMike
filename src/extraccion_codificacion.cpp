#include "extraccion_codificacion.h"

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <windows.h>
#include <stdexcept>
#include <iostream>
#include <thread>

using namespace cv;
using namespace std;

Mat screen_cap()
{
  IplImage *img;
  int nWidth;
  int nHeight;
  HWND hWnd;

  hWnd = GetDesktopWindow();
  if(hWnd == NULL)
  {
    throw std::runtime_error("error capturando escritorio");
  }
  /*
  if((hWnd =FindWindow( NULL, "Play Live Chess - Chess.com - Google Chrome")) == NULL)
  {
    printf("No encontre la ventana :(\n");
    return 0;
  }*/

  GetLastError();
  HDC hDC = GetDC(hWnd);
  HDC hMemDC = CreateCompatibleDC(hDC);
  RECT rect;

  if(GetWindowRect(hWnd, &rect) )
  {
    nWidth = rect.right - rect.left;
    nHeight = rect.bottom - rect.top;
  }

  HBITMAP hBitmap = CreateCompatibleBitmap( hDC, nWidth, nHeight);
  BITMAPINFO bmi;
  ZeroMemory(&bmi, sizeof(bmi));
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = nWidth;
  bmi.bmiHeader.biHeight = -(nHeight);
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biCompression = BI_RGB;
  bmi.bmiHeader.biSizeImage = 32 * nWidth * nHeight / 8;
  BYTE *pbBits = new BYTE[bmi.bmiHeader.biSizeImage];

  if( hBitmap)
  {
    HBITMAP hOld = (HBITMAP)SelectObject( hMemDC, hBitmap);
    std::cout << "ancho" << nWidth << " -- altura" <<nHeight << '\n';
    BitBlt( hMemDC, 0, 0, nWidth, nHeight, hDC, 0, 0, SRCCOPY);
    SelectObject( hMemDC, hOld);
    BITMAP bbmp;
    GetObject( hBitmap, sizeof(BITMAP), &bbmp);
    int nChannels = bbmp.bmBitsPixel == 1 ? 1 : bbmp.bmBitsPixel/8 ;
    int depth = bbmp.bmBitsPixel == 1 ? IPL_DEPTH_1U : IPL_DEPTH_8U;
    GetDIBits(hDC, hBitmap, 0, nHeight, pbBits, &bmi,DIB_RGB_COLORS);
    DeleteObject(hBitmap);
    img = cvCreateImageHeader( cvSize(bbmp.bmWidth, bbmp.bmHeight), depth, nChannels );
    img ->imageData = (char*)malloc(bbmp.bmHeight * bbmp.bmWidth * nChannels * sizeof(char));
    memcpy( img->imageData,(char*)(pbBits), bbmp.bmHeight * bbmp.bmWidth *
    nChannels);
  }

  delete[] pbBits;

  ::DeleteDC(hDC);
  ::DeleteDC(hMemDC);
  ::DeleteObject(hWnd);

  return Mat(img);
}

std::shared_ptr<vector<unsigned char>> codificar(cv::Mat& src, float factor_rsz=0.75)
{
  //cout << src.cols*src.rows*3 << '\t';
  vector<unsigned char> buffer;
  cvtColor(src, src, CV_BGR2GRAY);
  Size nvo_tam(src.cols*factor_rsz, src.rows*factor_rsz);
  resize(src,src,nvo_tam);
  vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    compression_params.push_back(50);

  cv::imencode(".jpg", src, (buffer),compression_params);
  cout << buffer.size() << '\t'; //217 es D9, lo cual es correcto. El end of frame es FFD9
  shared_ptr<vector<unsigned char>> copia = make_shared<vector<unsigned char>>(std::move(buffer)); //la ponemos bajo la custodia de un shared_ptr

  //Mat frame = cv::imdecode(buffer, CV_LOAD_IMAGE_COLOR);
  //cout << "\tframe w,h="<< frame.cols << "," <<frame.rows <<"\n";

  //imshow("cuadro", frame);
  //waitKey(15); //con 5 no le da tiempo de llenar el cuadro
  //thread t(mostrar, std::ref(frame));
  //t.detach();

  return copia;
  //return string(src.begin<unsigned char>(), src.end<unsigned char>());
}

cv::Mat frame_camara()
{
  static VideoCapture cap(0);
  Mat m;
  cap >> m;
  return m;
}

int solucionador_de_pedos(int i, const char* c1, const char* c2, const char* c3, int j, void* pf)
{
  return 0;
}
