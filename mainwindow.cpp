/*
* FIRWAR E.I.R.L
* Juan Carlos Agüero Flores
*/
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QMessageBox>

void MainWindow::usbport(void)
{
    usbCDC_is_available = false;
    usbCDC_port_name = "";
    usbCDC = new QSerialPort;

    //+- Solo para depurar
    qDebug()<< "number of available ports "<< QSerialPortInfo::availablePorts().length();
    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
    {
        //qDebug()<< "Has vendor ID" << serialPortInfo.hasVendorIdentifier();
        if (serialPortInfo.hasVendorIdentifier()){
            //qDebug()<< "Vendor id"<< serialPortInfo.vendorIdentifier();
        }

        //qDebug()<< "Has product ID" << serialPortInfo.hasProductIdentifier();
        if (serialPortInfo.hasProductIdentifier())
        {
            //qDebug()<< "Product ID" << serialPortInfo.productIdentifier();
        }
    }
    //+-
    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
    {
        if (serialPortInfo.hasProductIdentifier() && serialPortInfo.hasVendorIdentifier())
        {
            if ( (serialPortInfo.productIdentifier() == usbCDC_product_id) && (serialPortInfo.vendorIdentifier() == usbCDC_vendor_id) )
            {
                usbCDC_is_available = true;
                usbCDC_port_name  =serialPortInfo.portName();
                qDebug()<< usbCDC_port_name;
            }
        }
    }
    //+-
    if (usbCDC_is_available)
    {
        usbCDC->setPortName(usbCDC_port_name);
        //usbCDC->open(QSerialPort::WriteOnly);
        usbCDC->open(QSerialPort::ReadWrite);
        //usbCDC->open(QSerialPort::ReadOnly);//ok x lecturas
        usbCDC->setBaudRate(QSerialPort::Baud38400);
        usbCDC->setDataBits(QSerialPort::Data8);
        usbCDC->setParity(QSerialPort::NoParity);
        usbCDC->setStopBits(QSerialPort::OneStop);
        usbCDC->setFlowControl(QSerialPort::NoFlowControl);
        //
        QObject::connect(usbCDC, SIGNAL(readyRead()), this, SLOT(readSerial()));
    }
    else
    {
        QMessageBox::warning(this, "Port error", "Tarjeta de control no encontrado");
    }
}


void MainWindow::initChart(void)
{
    chart = new QChart();
    //chart->legend()->hide();
    chart->setTitle("Material resistance v1.0");
    //
    QValueAxis *axisX = new QValueAxis;
    axisX->setTitleText("Voltaje (mV)");
    axisX->setRange(-1500,1500);
    axisX->setTickCount(7);

    //QValueAxis *axisY = new QValueAxis;
    axisY = new QValueAxis;
    axisY->setTitleText("Distancia (m)");
    axisY->setRange(0,100);
    axisY->setTickCount(11);
    //caracteristicas de los ejes
    QPen penY1(Qt::darkBlue, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);//afecta la vis.del eje Y1
    axisY->setLinePen(penY1);//sobreescribe el color


    axisY->setGridLineColor(QColor(Qt::gray));//afecta la grilla
    axisY->setGridLineVisible(true);

    //![2] corriente
//    QValueAxis *axisXc = new QValueAxis;
//    axisXc->setTitleText("Corriente (mA)");
//    axisXc->setRange(0,500);
//    axisXc->setTickCount(5);
//    chart->addAxis(axisXc, Qt::AlignTop);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    //QSplineSeries *series = new QSplineSeries;
    //QLineSeries *series = new QLineSeries;
    //series = new QSplineSeries;
    series = new QLineSeries;

    //*series << QPointF(500, 2.5) << QPointF(-1000, 5) << QPointF(1500, 7.5) << QPointF(-1250, 10);
    *series << QPointF(0, 0);

    chart->addSeries(series);
    series->attachAxis(axisX);
    series->attachAxis(axisY);
    //
    ui->graphicsView->setChart(chart);
}
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    usbport();
    initChart();
}

MainWindow::~MainWindow()
{
    delete ui;

    if (usbCDC->isOpen())
    {
        usbCDC->close();
    }
}


/*
* Data payload: @dataASCII1,dataASCII2,..dataASCIIN\r\n
* el final de linea es compuesto por \r\n, pero solo uso \r para detectar el final como token de cierre
*/
#define TOKEN_BEGIN '@'
#define TOKEN_SEPARATOR ','
#define TOKEN_END '\r'
#define RXDATA_NUM_DATOS 3 //3 datos se envian
#define RXDATA_NUMMAX_CHARS_EN1DATO 20  //Numero max. de caracters en 1 dato esperado
#define RXDATA_NUM_TOKENS RXDATA_NUM_DATOS//Numero de tokens = # de datos que se envian

#if (RXDATA_NUM_DATOS == 0)
    #error "RXDATA_PAYLOAD_MAXSIZE = 0"
#endif

QString str_acc = "";
float meters = 0;
float volts = 0;
float current = 0;

void MainWindow::readSerial()
{
    static int counter = 0;
//    qDebug()<< "counter:" << counter++;

    QByteArray serialBuff = usbCDC->readAll();
    QString str_payload = QString::fromStdString(serialBuff.toStdString());
    str_acc += str_payload;
//    qDebug()<< str_acc;

    char c;
    char v[RXDATA_NUM_DATOS][RXDATA_NUMMAX_CHARS_EN1DATO];
    int sm0;
    int fil;
    int col;
    int counter_tokens = 0;
    char kc;

//    @120.1, 45, 555 \n
//    qDebug()<<"antes de buscar: " << QString::fromStdString(Cstr) << Qt::endl;
//    qDebug()<< "longitud"<< length << Qt::endl;

    sm0 = 0;
    fil = 0;
    col = 0;
    counter_tokens = 0;
    std::string Cstr = str_acc.toStdString();
    int length = Cstr.length();

    for (int i=0; i< length; i++)
    {
        c =  Cstr[i];
        //qDebug()<< i  << Qt::endl;
        if (sm0 == 0)
        {
            if ( c == TOKEN_BEGIN)
            {
                fil = 0;
                col = 0;

                #if (RXDATA_NUM_DATOS  == 1)
                    kc = TOKEN_END;
                #else
                    kc = TOKEN_SEPARATOR;
                #endif

                counter_tokens = 0;
                sm0++;
            }
        }
        else if (sm0 == 1)
        {
            if (c == kc)
            {
                v[fil][col] = '\0';

                #if (RXDATA_NUM_DATOS  > 1)
                    col = 0x0;
                    if (++fil == RXDATA_NUM_TOKENS-1)
                    {
                        kc = TOKEN_END; //'\n'
                    }
                #endif

                if (++counter_tokens == RXDATA_NUM_TOKENS)
                {
                    str_acc = "";
                    sm0 = 0;
                    break;
                }
            }
            else
            {
                v[fil][col++] = c;
            }
        }
    }
    if (counter_tokens == RXDATA_NUM_TOKENS)
    {
        counter_tokens = 0x00;

        //specific application
        meters = atof(&v[0][0]);
        volts = atof(&v[1][0]);
        current = atof(&v[2][0]);
        qDebug() << "meters: " <<meters;
        qDebug() << "volts: " << volts;
        qDebug() << "current: " << current;
        qDebug() << Qt::endl;
    //                    qDebug() << &v[0][0];
    //                    qDebug() << &v[1][0];
    //                    qDebug() << &v[2][0];
        //*series << QPointF(500, 2.5) << QPointF(-1000, 5) << QPointF(1500, 7.5) << QPointF(-1250, 10);
        //*series << QPointF(500, 2.5) << QPointF(-1000, 5) << QPointF(1500, 7.5) << QPointF(-1250, 10);
        *series << QPointF(volts, meters);
    }
}


void MainWindow::on_pushButton_clicked()
{
    axisY->setRange(0,50);
}
