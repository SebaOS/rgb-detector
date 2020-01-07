#include "dialog.h"
#include "ui_dialog.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QDateTime>
#include <QRegExp>


Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    arduino = new QSerialPort(this);
    this->addToLogs("Uruchomiono program.");
}

Dialog::~Dialog()
{
    if(arduino->isOpen()) {
        arduino->close();
    }
    delete ui;
}

/*!
 * \brief Dialog::makeConnect
 * Funkcja odpowiadająca za sprawdzenie czy Arduino jest połączone i przekazaniu informacji o znalezieniu urządzenia do textEditLogs
 */
void Dialog::makeConnect()
{
    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
        if(serialPortInfo.hasVendorIdentifier() && serialPortInfo.hasProductIdentifier()) {
            if(serialPortInfo.vendorIdentifier() == arduino_nano_vendor_id) {
                if(serialPortInfo.productIdentifier() == arduino_nano_product_id) {
                    arduino_is_available = true;
                    arduino_port_name = serialPortInfo.portName();
                    this->addToLogs("Znaleziono Arduino Nano");
                }
            }
        }
    }
}
/*!
 * \brief Dialog::openPort
 * Funkcja odpowiadająca za otwarcie portu z Arduino, gdy jest ono dostępne
 */
void Dialog::openPort()
{
    if(arduino_is_available) {
            this->addToLogs("Otwarto port Arduino... Połączono");
            arduino->setPortName(arduino_port_name);
            arduino->open(QSerialPort::ReadWrite);
            arduino->setBaudRate(QSerialPort::Baud9600);
            arduino->setDataBits(QSerialPort::Data8);
            arduino->setFlowControl(QSerialPort::NoFlowControl);
            arduino->setParity(QSerialPort::NoParity);
            arduino->setStopBits(QSerialPort::OneStop);
    } else {
            this->addToLogs("Nie udało się otworzyć portu / połączyć z Arduino");
    }
}
/*!
 * \brief Dialog::receiveData
 * Funckcja odpowiadająca za odbieranie danych z arduino i sprawdzanie ich pod kątem poprawności.
 * Jeżeli dane są poprawne to wywołuje funkcję, która je przetwarza.
 */
void Dialog::receiveData()
{
    while(this->arduino->canReadLine()) {
        line = this->arduino->readAll();
        terminator = "\r";
        pos = line.lastIndexOf(terminator);
        datas = line.left(pos);
        if(datas.contains(QRegExp("^\\w\\s(\\d+)\\s\\w\\s(\\d+)\\s\\w\\s(\\d+)$"))) {
            this->addToLogs(datas);
            splitDatas();
        }
    }
}
/*!
 * \brief Dialog::getColor
 * Funkcja wysyłająca informacje do Arduino, która powoduje odesłanie przez arduino danych.
 */
void Dialog::getColor()
{
    arduino->write("2");
}
/*!
 * \brief Dialog::addToLogs
 * @param message
 * Funckcja odpowiadająca za umieszczenie informacji w textEditLogs.
 * Jej parametr to informacja, która ma się tam znaleźć.
 */
void Dialog::addToLogs(QString message)
{
    QString currentDateTime = QDateTime::currentDateTime().toString("dd.MM.yy  hh:mm:ss");
    ui->textEditLogs->append(currentDateTime + "\t" + message);
}
/*!
 * \brief Dialog::splitDatas
 * Funckja odpowiadająca za pobranie ze Stringa wartości składowych RGB i ustawienie labelColor na kolor z tych składowych.
 */
void Dialog::splitDatas()
{
    QRegExp expr("(\\d+)");
    QVector<int> list;
    int pos = 0;
        while ((pos = expr.indexIn(datas, pos)) != -1) {
            list << expr.cap().toInt();
            pos += expr.matchedLength();
        }
        ui->labelColor->setStyleSheet(QString("background:rgb(%1,%2,%3);"
                                              "border: 4px solid qlineargradient(spread:pad, x1:0, y1:1, x2:1, y2:0, stop:0 rgba(151, 21, 255, 255), stop:1 rgba(118, 85, 112, 255));"
                                              "border-radius: 60px;").arg(list[0]).arg(list[1]).arg(list[2]));
}
/*!
 * \brief Dialog::on_detectButton_clicked
 * Po nacisnięci przycisku sprawdzany jest warunek czy połączenie z arduino jest otwarte.
 * Jeżeli połączenie nie jest otwarte, informacja o braku połączenia trafia do textEditLogs,
 * jeżeli jest to sprawdzany jest kolejny warunek, czyli czy checkBox odpowiadający za ciągłe pobieranie jest zaznaczony czy nie.
 * Zaznaczenie checkBoxa powoduje ciągłe pobieranie danych z czasem odstępu ustawionym w spinBoxie.
 * Niezaznaczenie checkBoxa przy otwartym połączeniu powoduje pobranie jednorazowo danych.
 */
void Dialog::on_detectButton_clicked()
{
    if(arduino->isOpen()) {
        if(!ui->checkBox->isChecked()) {
            getColor();
        } else if(ui->checkBox->isChecked()) {
            connect(timer, SIGNAL(timeout()), this, SLOT(getColor()));
            timer->start(ui->spinBox->value());
        }
    } else {
        this->addToLogs("Brak połączenia z Arduino");
    }
}
/*!
 * \brief Dialog::on_connectButton_clicked
 * Po nacisnięci przycisku sprawdzany jest warunek czy połączenie z arduino nie jest już otwarte.
 * Jeżeli jest to informacja o tym trafia do textEditLogs, jeżeli nie to port zostaje otwarty.
 */
void Dialog::on_connectButton_clicked()
{
    if(!arduino->isOpen()) {
    makeConnect();
    openPort();
    connect(this->arduino,SIGNAL(readyRead()), this, SLOT(receiveData()));
    } else {
       this->addToLogs("Połączono już z Arduino");
    }
}
/*!
 * \brief Dialog::on_disconnectButton_clicked
 * Po nacisnięci przycisku sprawdzany jest warunek czy połączenie z arduino jest otwarte.
 * Jeżeli jest to odbieranie informacji zostaje odłączone oraz połączenie z Arduino zamknięte,
 * jeżeli nie to informacja o tym trafia do textEditLogs.
 */
void Dialog::on_disconnectButton_clicked()
{
    if(arduino->isOpen()) {
        disconnect(this->arduino,SIGNAL(readyRead()), this, SLOT(receiveData()));
        arduino->close();
        this->addToLogs("Rozłączono z Arduino");
    } else {
        this->addToLogs("Nie ma połączenia z arduino");
    }
}
/*!
 * \brief Dialog::on_clearLogsButton_clicked
 * Po naciśnieciu przycisku textEditLogs zostaje wyczyszczony z logów.
 */
void Dialog::on_clearLogsButton_clicked()
{
    ui->textEditLogs->clear();
}
/*!
 * \brief Dialog::on_blackButton_clicked
 * Po nacisnięci przycisku sprawdzany jest warunek czy połączenie z arduino jest otwarte.
 * Jeżeli tak to wysyłana jest informacja do Arduino, która powoduje kalibrację koloru czarnego.
 */
void Dialog::on_blackButton_clicked()
{
    if(arduino->isOpen()) {
        arduino->write("0");
        this->addToLogs("Skalibrowano kolor czarny");
    } else {
        this->addToLogs("Połącz się najpierw z arduino!");
    }
}
/*!
 * \brief Dialog::on_whiteButton_clicked
 * Po nacisnięci przycisku sprawdzany jest warunek czy połączenie z arduino jest otwarte.
 * Jeżeli tak to wysyłana jest informacja do Arduino, która powoduje kalibrację koloru białego.
 */
void Dialog::on_whiteButton_clicked()
{
    if(arduino->isOpen()) {
        arduino->write("1");
        this->addToLogs("Skalibrowano kolor biały");
    } else {
        this->addToLogs("Połącz się najpierw z arduino!");
    }
}
/*!
 * \brief Dialog::on_stopButton_clicked
 * Po nacisnięci przycisku zatrzymane zostaje ciągłe pobieranie danych.
 */
void Dialog::on_stopButton_clicked()
{
    disconnect(timer, SIGNAL(timeout()), this, SLOT(getColor()));
}




