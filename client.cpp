#include "client.h"
#include "ui_client.h"
#include <QMessageBox>
#include <QFileDialog>

Client::Client(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Client),
    socket(new QTcpSocket(this))
{
    ui->setupUi(this);
    connect(socket,&QTcpSocket::readyRead,this,&Client::readMsg);
    typedef void (QAbstractSocket::*QAbstractSocketErrorSignal)(QAbstractSocket::SocketError);
    connect(socket, static_cast<QAbstractSocketErrorSignal>(&QAbstractSocket::error),
            this, &Client::error);
    connect(socket,&QTcpSocket::connected,this,&Client::connected);
    connect(socket,&QTcpSocket::disconnected,this,&Client::disconnected);
    ui->disconnect->setDisabled(true);
    ui->open_file->setDisabled(true);
    ui->send_file->setDisabled(true);
    ui->save->setDisabled(true);
    ui->sendpro->setMaximum(100);
    ui->sendpro->reset();
    QFile config("config.ini");
    config.open(QFile::ReadOnly);
    defaultpath = QString(config.readLine());
    qDebug()<<defaultpath;
    config.close();
}

Client::~Client()
{
    delete ui;
    if(file != NULL)
    {
        file->close();
        delete file;
    }
}

void Client::sendfile()
{
    ui->message->append("start send file");
    ui->send_file->setDisabled(true);
    ui->open_file->setDisabled(true);
    int bytenum=1024*1024;
    int waittosend = file->size();
    QByteArray outblock;
    ui->sendpro->setValue(0);
    while(waittosend != 0)
    {
        if(waittosend < bytenum)
        {
            bytenum = waittosend;
            waittosend = 0;
        }else{
            waittosend -= bytenum;
        }
        outblock = file->read(bytenum);
        socket->write(outblock);
        socket->flush();
        ui->sendpro->setValue((1-waittosend*1.0/file->size())*90);
        qDebug()<<(1-waittosend*1.0/file->size())*90;
    }
    ui->send_file->setDisabled(false);
    ui->open_file->setDisabled(false);
}

void Client::readMsg()
{
    QByteArray qba = socket->readAll();
    qDebug()<<qba;
    QString ss=QVariant(qba).toString();
    ui->message->append(ss);
    if(ss=="OK")
        sendfile();
    else if(ss=="recv file finished")
        ui->sendpro->setValue(100);
}

void Client::error(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
      case QAbstractSocket::RemoteHostClosedError:
          break;
      case QAbstractSocket::HostNotFoundError:
          QMessageBox::information(this, tr("Fortune Client"),
                                   tr("The host was not found. Please check the "
                                      "host name and port settings."));
          break;
      case QAbstractSocket::ConnectionRefusedError:
          QMessageBox::information(this, tr("Fortune Client"),
                                   tr("The connection was refused by the peer. "
                                      "Make sure the fortune server is running, "
                                      "and check that the host name and port "
                                      "settings are correct."));
          break;
      default:
          QMessageBox::information(this, tr("Fortune Client"),
                                   tr("The following error occurred: %1.")
                                   .arg(socket->errorString()));
      }
}

void Client::on_connect_clicked()
{
    QString ipaddr = ui->ipaddr->text();
    QString port = ui->port->text();
    if(ipaddr.isEmpty() || port.isEmpty())
    {
        QMessageBox::warning(this,QString::fromLocal8Bit("出错"),QString::fromLocal8Bit("主机地址和端口号不能为空！"));
        return ;
    }
    socket->abort();
    socket->connectToHost(ipaddr,port.toUShort());
}

void Client::connected()
{
    ui->message->append(QString::fromLocal8Bit("已连接。"));
    ui->connect->setDisabled(true);
    ui->disconnect->setDisabled(false);
    ui->open_file->setDisabled(false);
}

void Client::disconnected()
{
    ui->message->append(QString::fromLocal8Bit("已断开。"));
    ui->connect->setDisabled(false);
    ui->disconnect->setDisabled(true);
    ui->open_file->setDisabled(true);
    ui->send_file->setDisabled(true);
    ui->save->setDisabled(true);
}

void Client::on_pushButton_clicked()
{
    ui->message->clear();
}

void Client::on_disconnect_clicked()
{
    socket->write("stop");
}

void Client::on_open_file_clicked()
{
    fileName = QFileDialog::getOpenFileName(this, tr("open file"), defaultpath,  tr("HEX(*.hex);;ALL(*.*)"));
    if(fileName.isEmpty())
        return;
    ui->file_name->setText(fileName.split('/').last());
    defaultpath = fileName;
    ui->send_file->setDisabled(false);
    ui->save->setDisabled(false);
    ui->sendpro->reset();
}

void Client::on_send_file_clicked()
{
    QByteArray fileinfo;
    fileinfo = "@start,";
    if(file != NULL)
    {
        file->close();
        delete file;
    }
    file = new QFile(fileName);
    file->open(QFile::ReadOnly);
    fileinfo += QByteArray::number(file->size()) + ",";
    QString name = QString(fileName.split('/').last());
    fileinfo += name;
    socket->write(fileinfo);
}

void Client::on_save_clicked()
{
    QFile config("config.ini");
    config.open(QFile::WriteOnly);
    config.write(defaultpath.toLocal8Bit());
    config.close();
}
