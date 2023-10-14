#ifndef TCP_THREAD_H
#define TCP_THREAD_H

#include <QThread>
#include <QHostAddress>
#include <QQueue>
#include <QMutex>
#include <QTcpSocket>
#include <QTcpServer>

class Tcp_Thread : public QThread
{
    Q_OBJECT
public:
    explicit Tcp_Thread(QHostAddress ip,QString port,QObject *parent = nullptr);
    ~Tcp_Thread();

    bool start_flag; //线程工作标志位
    bool connect_flag; //tcp连接标志位
    bool recv_flag; //保存接收数据到队列标志位
    QQueue<QByteArray> bytes_queue;
    QQueue<QString> msg_queue;

    QTcpSocket *socket; //套接字
    QTcpServer *server; //
    QHostAddress client_ip;
    qint16 client_port;

    void stop(); //暂停线程
    void go_on(); //继续线程
    void new_client_connect();

protected:
    void run();

signals:
    void signal_msg();
private:
    QMutex mutex; //互斥量，用来暂停和继续线程

private slots:
    void read_tcp();
    void socket_disconnect();

};

#endif // TCP_THREAD_H
