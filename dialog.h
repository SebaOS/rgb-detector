#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QSerialPort>
#include <QTimer>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);

    ~Dialog();

private slots:
    void makeConnect();

    void openPort();

    void receiveData();

    void getColor();

    void addToLogs(QString message);

    void splitDatas();

    void on_detectButton_clicked();

    void on_connectButton_clicked();

    void on_disconnectButton_clicked();

    void on_clearLogsButton_clicked();

    void on_blackButton_clicked();

    void on_whiteButton_clicked();

    void on_stopButton_clicked();

private:
    Ui::Dialog *ui;
    QSerialPort *arduino;
    static const quint16 arduino_nano_vendor_id = 6790;
    static const quint16 arduino_nano_product_id = 29987;
    QTimer *timer = new QTimer(this);
    QString arduino_port_name;
    QString line;
    QString terminator;
    QString datas;
    int pos;
    bool arduino_is_available;
    bool interrupt;
};

#endif // DIALOG_H
