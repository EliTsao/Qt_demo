#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QHostAddress>

#include "udp_thread.h"
#include "tcp_thread.h"


QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;
    Udp_Thread *udp_thread;
    Tcp_Thread *tcp_thread;

    QHostAddress get_local_host_ip();
    void is_ui_visiable(bool state);

private slots:
    void close_thread();
    void display_udp_frame();
    void print_tcp_msg();

    void on_btn_open_clicked();
    void on_btn_path_change_clicked();
    void on_btn_snap_clicked();
    void on_btn_screen_cap_clicked();
    void on_btn_clear_clicked();
    void on_btn_send_clicked();
};
#endif // WIDGET_H
