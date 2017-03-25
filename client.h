#ifndef CLIENT_H
#define CLIENT_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QFile>

namespace Ui {
class Client;
}

class Client : public QMainWindow
{
    Q_OBJECT

public:
    explicit Client(QWidget *parent = 0);
    ~Client();

private:
    Ui::Client *ui;
    QTcpSocket *socket;
    QString fileName;
    QFile *file;
    QString defaultpath;

    void sendfile();

private slots:
    void readMsg();
    void error(QAbstractSocket::SocketError socketError);
    void on_connect_clicked();
    void connected();
    void disconnected();
    void on_pushButton_clicked();
    void on_disconnect_clicked();
    void on_open_file_clicked();
    void on_send_file_clicked();
    void on_save_clicked();
};

#endif // CLIENT_H
