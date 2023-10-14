#include "tcp_thread.h"
#include <QDebug>

Tcp_Thread::Tcp_Thread(QHostAddress ip,QString port,QObject *parent) : QThread(parent)
{
    start_flag = true; //线程工作标志位
    connect_flag = false; //TCP连接标志位
    recv_flag = false;

    server = new QTcpServer(); //新建一个对象
    server->listen(ip, port.toInt()); //监听端口，等待客户端连接
    connect(server,&QTcpServer::newConnection,this,&Tcp_Thread::new_client_connect);
}


void Tcp_Thread::run()
{
    QByteArray data_temp;
    while(start_flag)
    {
        mutex.lock();
        if(!bytes_queue.isEmpty())
        {
            data_temp = bytes_queue.dequeue();
            msg_queue.enqueue(QString(data_temp));
            emit signal_msg();
        }
        mutex.unlock();
    }
}

void Tcp_Thread::new_client_connect()
{
    socket = server->nextPendingConnection(); //获取客户端套接字
    client_ip = socket->peerAddress();
    client_port = socket->peerPort(); //此时为空
    connect(socket,SIGNAL(readyRead()),this,SLOT(read_tcp()));
    connect(socket, SIGNAL(disconnected()),this, SLOT(socket_disconnect()));
    connect_flag = true;
    msg_queue.enqueue(QString("上线 [%1]").arg(client_ip.toString()));
    emit signal_msg();
}

//读取客户端发送来的数据
void Tcp_Thread::read_tcp()
{
    QByteArray buffer;
    buffer = socket->readAll();
    if(recv_flag)
    {
        bytes_queue.enqueue(buffer);
    }
}

void Tcp_Thread::socket_disconnect()
{
    connect_flag = false;
    msg_queue.enqueue(QString("已断开！"));
    emit signal_msg();
}

void Tcp_Thread::stop()
{
    mutex.lock();
}

void Tcp_Thread::go_on()
{
    qDebug()<<connect_flag<<"go on";
    mutex.unlock();
}

Tcp_Thread::~Tcp_Thread()
{

}
