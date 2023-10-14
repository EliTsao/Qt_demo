#include "widget.h"
#include "ui_widget.h"
#include <QDir>
#include <QNetworkInterface>
#include <QFileDialog>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowTitle("接收端（服务端）");

    //作为服务端，获取本机ip地址，将UDP端口设为8888
    QHostAddress local_ip = get_local_host_ip();
    QString udp_port = "8888";
    QString tcp_port = "9999";

    ui->lineEdit_path->setText(QDir::currentPath()+"/"); //添加项目文件路径到控件
    ui->label_ip_addr->setText(local_ip.toString());
    ui->label_udp_port->setText(udp_port);
    ui->label_tcp_port->setText(tcp_port);
    is_ui_visiable(false);

    //新建TCP对象和thread对象
    tcp_thread = new Tcp_Thread(local_ip,tcp_port);
    connect(tcp_thread,&Tcp_Thread::signal_msg,this,&Widget::print_tcp_msg);

    //新建udp对象和thread对象
    udp_thread = new Udp_Thread(local_ip,udp_port);
    connect(udp_thread, &Udp_Thread::signal_img, this, &Widget::display_udp_frame); //接收线程信号

    connect(this, &Widget::destroyed, this, &Widget::close_thread); //线程跟随窗口退出

    tcp_thread->start();
    udp_thread->start();
    tcp_thread->stop();
    udp_thread->stop(); //暂停udp线程，等到开启接收画面后再开启工作
    ui->label_msg->setText("等待客户端连接...");
}


//显示图片、视频
void Widget::display_udp_frame()
{
    if(!udp_thread->pixmap_queue.isEmpty())
    {
        QPixmap pix = udp_thread->pixmap_queue.dequeue();
        ui->label_frame->setPixmap(pix.scaled(ui->label_frame->size(),  Qt::KeepAspectRatio));
        if(!udp_thread->save_img_result.isEmpty())
        {
            ui->label_msg->append("已保存！"+udp_thread->save_img_result);
            udp_thread->save_img_result.clear();
        }
        else if(udp_thread->save_video_end_flag)
        {
            ui->label_msg->append("已保存！"+udp_thread->save_video_result);
            udp_thread->save_video_end_flag = false;
        }
    }
}

//显示TCP数据
void Widget::print_tcp_msg()
{
    qDebug()<<tcp_thread->connect_flag<<"msg";
    if(!tcp_thread->msg_queue.isEmpty())
    {
        ui->label_msg->setTextColor(QColor::fromRgb(255,0,0));
        ui->label_msg->append("client: "+tcp_thread->msg_queue.dequeue());
        ui->label_msg->setTextColor(QColor::fromRgb(0,0,0));
        if(!tcp_thread->connect_flag)
        {
            on_btn_open_clicked();
            ui->label_msg->setText("等待客户端连接...");
        }
    }
}

//关闭线程
void Widget::close_thread()
{
    tcp_thread->go_on();
    tcp_thread->start_flag = false; //停止线程
    tcp_thread->quit();
    tcp_thread->wait(); //等待线程处理完手头工作
    if(tcp_thread->connect_flag)
    {
        tcp_thread->socket->abort();
        tcp_thread->server->close();
    }

    udp_thread->go_on();
    udp_thread->start_flag = false; //停止线程
    udp_thread->quit();
    udp_thread->wait(); //等待线程处理完手头工作
}


//开启监听
void Widget::on_btn_open_clicked()
{
    qDebug()<<tcp_thread->connect_flag<<"hh";
    if(ui->btn_open->text().toUtf8() == "开启监听")
    {
        if(tcp_thread->connect_flag)
        {
            ui->btn_open->setText("关闭监听");
            is_ui_visiable(true); //控件使能
            udp_thread->go_on(); //继续udp线程
            tcp_thread->go_on(); //继续tcp线程
            udp_thread->recv_flag = true;
            tcp_thread->recv_flag = true;
        }
        else
        {
            ui->label_msg->append("无客户端连接！");
        }
    }
    else if(ui->btn_open->text().toUtf8() == "关闭监听")
    {
        ui->btn_open->setText("开启监听");
        udp_thread->recv_flag = false;
        tcp_thread->recv_flag = false;
        is_ui_visiable(false); //控件失能
        udp_thread->stop();  //暂停udp线程
        tcp_thread->stop(); //暂停udp线程
        ui->label_msg->clear();
        ui->label_frame->clear(); //清空视频页面
    }
}

void Widget::on_btn_send_clicked()
{
    QString msg = ui->textEdit_send->toPlainText();
    tcp_thread->socket->write(msg.toUtf8());
    tcp_thread->socket->flush();
    ui->label_msg->setTextColor(QColor::fromRgb(0,0,255));
    ui->label_msg->append("server: "+msg);
    ui->label_msg->setTextColor(QColor::fromRgb(0,0,0));
}

//截屏
void Widget::on_btn_snap_clicked()
{
    if(ui->btn_jpg->isChecked())
    {
        udp_thread->save_img_type = ".jpg";
    }
    else if(ui->btn_png->isChecked())
    {
        udp_thread->save_img_type = ".png";
    }
    udp_thread->save_img_path = ui->lineEdit_path->text().toUtf8();
    udp_thread->save_img_flag = true;
}


//录屏
void Widget::on_btn_screen_cap_clicked()
{
    if(ui->btn_screen_cap->text().toUtf8() == "录屏")
    {
        ui->btn_screen_cap->setText("停止录屏");
        udp_thread->save_video_path = ui->lineEdit_path->text().toUtf8();
        udp_thread->save_video_start_flag = true;
    }
    else if(ui->btn_screen_cap->text().toUtf8() == "停止录屏")
    {
        ui->btn_screen_cap->setText("录屏");
        udp_thread->save_video_end_flag = true;
    }

}


//更改文件保存路径
void Widget::on_btn_path_change_clicked()
{
    QString filePath = QFileDialog::getExistingDirectory(this, "请选择文件保存路径…", "./");
    if(filePath.isEmpty()) filePath = ".";
    ui->lineEdit_path->setText(filePath+"/");
}

//清空接受区
void Widget::on_btn_clear_clicked()
{
    ui->label_msg->clear();
}

//控件是否可操作
void Widget::is_ui_visiable(bool state)
{
    ui->btn_snap->setEnabled(state);
    ui->btn_screen_cap->setEnabled(state);
    ui->lineEdit_path->setEnabled(state);
    ui->btn_path_change->setEnabled(state);
    ui->btn_jpg->setEnabled(state);
    ui->btn_png->setEnabled(state);
    ui->btn_clear->setEnabled(state);
    ui->btn_send->setEnabled(state);
    ui->textEdit_send->setEnabled(state);
}

//获取本机在局域网下的ip地址
QHostAddress Widget::get_local_host_ip()
{
  QList<QHostAddress> AddressList = QNetworkInterface::allAddresses();
  QHostAddress result;
  foreach(QHostAddress address, AddressList){
      if(address.protocol() == QAbstractSocket::IPv4Protocol &&
         address != QHostAddress::Null &&
         address != QHostAddress::LocalHost){
          if (address.toString().contains("127.0.")){
            continue;
          }

          if(address.toString().contains("192.168.1.")){
            result = address;
            break;
          }
      }
  }
  return result;
}

Widget::~Widget()
{
    delete ui;
}
