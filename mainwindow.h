#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QSerialPort;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void usbport(void);

private slots:
    void readSerial();

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
