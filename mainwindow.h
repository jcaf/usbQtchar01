#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QtCharts>//faltaba incluir este header
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QCategoryAxis>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QSerialPort;

//class QTCharts;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void usbport(void);

    //////////////////////////
    QChart *chart;
    //QSplineSeries *series;
    QLineSeries *series;

    QValueAxis *axisY;
    //////////////////////
    void initChart(void);

private slots:
    void readSerial();

    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;


    //
    QSerialPort *usbCDC;
    static const quint16 usbCDC_vendor_id = 9025;//0x2341
    static const quint16 usbCDC_product_id = 67;//0x0047
    QString usbCDC_port_name;
    bool usbCDC_is_available;
};
#endif // MAINWINDOW_H
