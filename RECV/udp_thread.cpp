#include "udp_thread.h"
#include <QBuffer>
#include <QImageReader>
#include <QDateTime>

Udp_Thread::Udp_Thread(QHostAddress ip, QString port, QObject *parent) : QThread(parent)
{
    start_flag = true; //默认开启线程
    save_img_flag = false; //默认不保存图片
    recv_flag = false;
    save_video_start_flag = false; //默认不保存视频
    save_video_end_flag = false;

    socket = new QUdpSocket(); //新建一个udp套接字对象
    socket->bind(ip,port.toInt()); //绑定ip地址并监听端口
    connect(socket,SIGNAL(readyRead()),this,SLOT(recv_udp()));
}

void Udp_Thread::run()
{
    QByteArray img_bytes;
    QString current_time;
    while(start_flag)
    {
        mutex.lock();
        if(!bytes_queue.isEmpty())
        {
            img_bytes = bytes_queue.dequeue();
            QBuffer img_buffer(&img_bytes);
            QImageReader img_reader(&img_buffer,"JPEG");
            QPixmap pix = QPixmap::fromImage(img_reader.read());
            pixmap_queue.enqueue(pix);
            emit signal_img();
            img_bytes.clear();
            if(save_img_flag)
            {
                current_time = QDateTime::currentDateTime().toString \
                                        ("MM-dd-hh-mm-ss");
                save_img_result = save_img_path+current_time+save_img_type;
                pix.save(save_img_result);
                save_img_flag = false;
            }
            if(save_video_start_flag)
            {
                current_time = QDateTime::currentDateTime().toString \
                        ("MM-dd-hh-mm-ss");
                save_video_result = save_video_path + current_time + ".avi";
                save_video_writer.open(save_video_result.toStdString(),VideoWriter::fourcc( \
                                           'M','J','P','G'), 30.0, Size(640, 480), true);
                save_video_start_flag = false;
            }
            else if(save_video_end_flag && !save_video_result.isEmpty())
            {
                save_video_writer.release();
            }
            if(save_video_writer.isOpened() && !save_video_end_flag)
            {
                save_video_writer.write(QImageToMat(pix.toImage()));
            }
        }
        mutex.unlock();
    }
}

//UDP接收图像数据
void Udp_Thread::recv_udp()
{
    QByteArray buff_temp;
    buff_temp.resize(socket->bytesAvailable());
    socket->readDatagram(buff_temp.data(),buff_temp.size());
    if(recv_flag)
        bytes_queue.enqueue(buff_temp); //加入队列，等待线程处理
}

//QImage转Mat，使用opencv库函数进行保存视频
Mat Udp_Thread::QImageToMat(QImage image)
{
    Mat mat;
    switch (image.format())
    {
        case QImage::Format_ARGB32:
        case QImage::Format_RGB32:
        case QImage::Format_ARGB32_Premultiplied:
            mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
            break;
        case QImage::Format_RGB888:
            mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
            cv::cvtColor(mat, mat, CV_BGR2RGB);
            break;
        case QImage::Format_Grayscale8:
            mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
            break;
        default:
            break;
    }
    return mat;
}

void Udp_Thread::stop()
{
    mutex.lock();
}

void Udp_Thread::go_on()
{
    mutex.unlock();
}

Udp_Thread::~Udp_Thread()
{

}
