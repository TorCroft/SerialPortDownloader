#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_SerialPortDownloader.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSettings>
#include <QFileDialog>
#include <QtGui>
#include "ImageProcess.h"


class SerialPortDownloader : public QMainWindow
{
    Q_OBJECT

public:
    SerialPortDownloader(QWidget *parent = nullptr);
    ~SerialPortDownloader();

private:
    Ui::SerialPortDownloaderClass ui;
    QSerialPort serialport;
    Mat img_raw;
    QByteArray hexData;
    void show_preview_image(Mat);

private slots:
    void on_baudRateComboBox_currentTextChanged();
    void on_portComboBox_currentTextChanged();
    void on_selectImageButton_clicked();
    void on_portScanButton_clicked();
    void on_portSendButton_clicked();
    void on_openPortButton_clicked();
    void on_binarizeButton_clicked();
    void on_syncButton_clicked();
    void on_clearButton_clicked();
};
