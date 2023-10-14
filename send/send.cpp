#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <ctime>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <unistd.h>  // 添加此头文件以支持 close 函数
#include <netdb.h>   // 添加此头文件以支持 NI_MAXHOST 和 getnameinfo 函数
#include <sys/types.h>

const int UDP_PORT = 8888;
const int TCP_PORT = 9999;

bool tcp_connect_flag = false;

class UDP_SOCKET {
public:
    struct sockaddr_in serverAddr; // 声明为公有成员

    UDP_SOCKET(const char* server_addr, int port = UDP_PORT) {
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = inet_addr(server_addr);
        udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    }

    void send_frame(const cv::Mat& frame, const struct sockaddr_in& addr) {
        std::vector<uchar> encode_img;
        cv::imencode(".jpg", frame, encode_img, {cv::IMWRITE_JPEG_QUALITY, 48});

        ssize_t sent_bytes = sendto(udpSocket, encode_img.data(), encode_img.size(), 0, (struct sockaddr*)&addr, sizeof(addr));
        if (sent_bytes == -1) {
            perror("Error sending data");
        }
    }

    void closeSocket() {
        ::close(udpSocket);  // 使用全局作用域的 close 函数
    }

private:
    int udpSocket;
};

class TCP_SOCKET {
public:
    TCP_SOCKET(const char* server_addr, int port = TCP_PORT) {
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(server_addr);
        tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
        connectTCP();
    }


    void connectTCP() {
        std::cout << "正在连接服务器..." << std::endl;
        while (true) {
            if (connect(tcpSocket, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
                std::cout << "tcp connect success!" << std::endl;
                tcp_connect_flag = true;
                break;
            }
        }
    }

void send_data() {
    while (tcp_connect_flag) {
        // Get data (replace these lines with actual data retrieval)
        int amount_person = 5;     // Get amount_person data
        int amount_car = 10;       // Get amount_car data
        int amount_bullseye = 3;   // Get amount_bullseye data

        // Convert integers to string format and concatenate them with a delimiter
        std::string dataString = std::to_string(amount_person) + " " +
                                 std::to_string(amount_car) + " " +
                                 std::to_string(amount_bullseye);

        // Send the data over the socket
        ssize_t sent_bytes = send(tcpSocket, dataString.c_str(), dataString.size(), 0);
        if (sent_bytes == -1) {
            perror("Error sending data");
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}



    void recv_data() {
        while (true) {
            char data[1024] = {0};
            ssize_t bytes_received = recv(tcpSocket, data, sizeof(data), 0);
            if (bytes_received > 0) {
                std::cout << "server: " << data << std::endl;
            } else {
                std::cout << "服务端已退出!" << std::endl;
                tcp_connect_flag = false;
                break;
            }
        }
    }

    void closeSocket() {
        ::close(tcpSocket);  // 使用全局作用域的 close 函数
    }

private:
    int tcpSocket;
    struct sockaddr_in addr;
};

class IMAGE_FUNC {
public:
    UDP_SOCKET udp;
    TCP_SOCKET tcp;

    IMAGE_FUNC(const char* udp_server_addr, const char* tcp_server_addr)
        : udp(udp_server_addr), tcp(tcp_server_addr) {}

    void send_video() {
        cv::VideoCapture cam(0);
        if (cam.isOpened()) {
            while (true) {
                cv::Mat frame;
                cam >> frame;
                if (frame.empty()) {
                    std::cout << "cam error!" << std::endl;
                    break;
                }

                udp.send_frame(frame, udp.serverAddr);

                char key = cv::waitKey(20);
                if (key == 'q') {
                    break;
                } else if (!tcp_connect_flag) {
                    std::cout << "tcp 已断开连接！" << std::endl;
                    break;
                }
            }
        } else {
            std::cout << "cam error!" << std::endl;
        }
        cam.release();
    }

    void send_local_video(const std::string& video_path) {
        cv::VideoCapture videoCapture(video_path);
        if (!videoCapture.isOpened()) {
            std::cout << "Video file not found!" << std::endl;
            return;
        }
        while (true) {
            cv::Mat frame;
            videoCapture >> frame;

            if (frame.empty()) {
                std::cout << "End of video!" << std::endl;
                break;
            }

            udp.send_frame(frame, udp.serverAddr);

            char key = cv::waitKey(20);
            if (key == 'q') {
                break;
            } else if (!tcp_connect_flag) {
                std::cout << "tcp 已断开连接！" << std::endl;
                break;
            }
        }

        videoCapture.release();
    }
};

std::string getLocalIP() {
    struct ifaddrs *ifaddr, *ifa;
    char host[NI_MAXHOST] = "";

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) {
            continue;
        }

        int family = ifa->ifa_addr->sa_family;
        if (family == AF_INET) {
            int s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                                host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s == 0 && std::string(ifa->ifa_name) == "eth0") {
                break;
            }
        }
    }

    freeifaddrs(ifaddr);
    return host;
}

int main() {
    // 获取本地IP地址
    std::string localIP = getLocalIP();
    std::cout << "Local IP: " << localIP << std::endl;

    // 创建UDP_SOCKET和TCP_SOCKET实例
    IMAGE_FUNC img(localIP.c_str(), localIP.c_str());

    // 创建并启动线程
    std::thread tcp_send_thread(&TCP_SOCKET::send_data, &img.tcp);
    std::thread tcp_recv_thread(&TCP_SOCKET::recv_data, &img.tcp);
    std::thread udp_video_thread(&IMAGE_FUNC::send_local_video, &img, "test1.mp4");

    // 等待线程完成
    udp_video_thread.join();
    tcp_send_thread.detach();
    tcp_recv_thread.detach();

    // 关闭UDP和TCP连接
    img.udp.closeSocket();
    img.tcp.closeSocket();

    // 销毁OpenCV窗口
    cv::destroyAllWindows();

    return 0;
}
