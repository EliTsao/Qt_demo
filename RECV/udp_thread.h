#ifndef UDP_THREAD_H
#define UDP_THREAD_H

#include <QThread>
#include <QQueue>
#include <QPixmap>
#include <QMutex>

#include <QUdpSocket>
#include <QHostAddress>

#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/types_c.h>

using namespace cv; ///OpenCV命名空间

class Udp_Thread : public QThread
{
    Q_OBJECT
public:
    explicit Udp_Thread(QHostAddress ip,QString port,QObject *parent = nullptr);
    ~Udp_Thread();

    bool start_flag; //线程工作标志位
    bool recv_flag;  //保存udp数据到队列标志位
    QQueue<QByteArray> bytes_queue; //图像原始数据队列
    QQueue<QPixmap> pixmap_queue; //图像数据队列

    //图片保存相关
    bool save_img_flag;
    QString save_img_type;
    QString save_img_path;
    QString save_img_result;

    //保存视频相关
    bool save_video_start_flag;
    bool save_video_end_flag;
    VideoWriter save_video_writer;
    QString save_video_path;
    QString save_video_result;

    QUdpSocket *socket; //udp套接字

    void stop(); //暂停线程
    void go_on(); //继续线程

protected:
    void run();

signals:
   void signal_img();

private:
    QMutex mutex;//互斥量，用来暂停和继续线程
    Mat QImageToMat(QImage image);

private slots:
    void recv_udp();

};

#endif // UDP_THREAD_H
