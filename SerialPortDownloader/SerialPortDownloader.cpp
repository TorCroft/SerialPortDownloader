#include "SerialPortDownloader.h"

QImage stringToImage(const QString& str)
{
    QImage image(1000, 1000, QImage::Format_Grayscale8);
    image.fill(Qt::white);
    if (str.isEmpty())
    {
        return image.scaled(QSize(200, 200), Qt::KeepAspectRatio);
    }
    QTextOption option;
    QRect rect(0, 0, 1000, 1000);
    QPainter painter(&image);
    painter.setFont(QFont("黑体", 100));
    painter.setPen(Qt::black);
    painter.setRenderHint(QPainter::Antialiasing, true); //抗锯齿
    // 设置文本对齐方式为左对齐、自动换行
    option.setAlignment(Qt::AlignLeft | Qt::AlignTop);
    option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    
    QRectF textRect = painter.boundingRect(rect, str, option);
    if (!rect.contains(textRect.toRect()))
    {
        QMessageBox::warning(nullptr, "Text is too long", "The text cannot be fully displayed within the specified area.");
    }
    else
    {
        painter.drawText(rect, str, option);
    }
    return image.scaled(QSize(200, 200), Qt::KeepAspectRatio);
}


SerialPortDownloader::SerialPortDownloader(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    connect(&serialport, &QSerialPort::errorOccurred, [=](QSerialPort::SerialPortError error) {
        if (error == QSerialPort::ResourceError && !serialport.isOpen()) {
            ui.openPortButton->setText("打开串口");
            ui.baudRateComboBox->setEnabled(true);
            ui.portComboBox->setEnabled(true);
            ui.portSendButton->setEnabled(false);
        }
    });

    serialport.setBaudRate(QSerialPort::Baud115200);
    serialport.setDataBits(QSerialPort::Data8);
    serialport.setParity(QSerialPort::NoParity);
    serialport.setStopBits(QSerialPort::OneStop);
    serialport.setFlowControl(QSerialPort::NoFlowControl);
    on_syncButton_clicked();
}

SerialPortDownloader::~SerialPortDownloader(){}

void SerialPortDownloader::on_portScanButton_clicked()
{
    ui.portComboBox->clear();
    foreach(const QSerialPortInfo & info, QSerialPortInfo::availablePorts())
    {
        ui.portComboBox->addItem(info.portName());
    }
    if (ui.portComboBox->count())
    {
        ui.openPortButton->setEnabled(true);
    }
    else
    {
        ui.openPortButton->setEnabled(false);
    }
}

void SerialPortDownloader::on_portComboBox_currentTextChanged()
{
    serialport.setPortName(ui.portComboBox->currentText());
}


void SerialPortDownloader::on_baudRateComboBox_currentTextChanged()
{
    serialport.setBaudRate(ui.baudRateComboBox->currentText().toInt());
}

void SerialPortDownloader::on_openPortButton_clicked()
{
    if (serialport.isOpen())
    {
        serialport.close();
        ui.openPortButton->setText("打开串口");
        ui.baudRateComboBox->setEnabled(true);
        ui.portComboBox->setEnabled(true);
        ui.portScanButton->setEnabled(true);
        ui.portSendButton->setEnabled(false);
    }
    else
    {
        ui.openPortButton->setText("关闭串口");
        serialport.open(QIODevice::ReadWrite);
        ui.baudRateComboBox->setEnabled(false);
        ui.portComboBox->setEnabled(false);
        ui.portScanButton->setEnabled(false);
        ui.portSendButton->setEnabled(true);
    }  
}

void SerialPortDownloader::on_clearButton_clicked()
{
    ui.plainTextEdit->clear();
}


void SerialPortDownloader::on_portSendButton_clicked()
{
    serialport.write(hexData);
}

void SerialPortDownloader::show_preview_image(Mat image)
{
    image = convertToRGB(image);
    QImage qimage(image.data, image.cols, image.rows, QImage::Format_RGB888);
    ui.previewLabel->setPixmap(QPixmap::fromImage(qimage));
}

void SerialPortDownloader::on_selectImageButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), QDir::homePath(), tr("Images (*.png *.xpm *.jpg)"));
    if (!fileName.isEmpty())
    {
        img_raw = imread(fileName.toLocal8Bit().toStdString()); //imread 不能直接读取含有中文字符的路径，需要借助toLocal8Bit().toStdString()
        cv::resize(img_raw, img_raw, Size(200, 200));
        show_preview_image(img_raw);
        ui.binarizeButton->setEnabled(true);
    }
    else if(img_raw.empty())
    {
        ui.binarizeButton->setEnabled(false);
    }
}


void SerialPortDownloader::on_syncButton_clicked()
{
    QImage qBinaryImg = stringToImage(ui.plainTextEdit->toPlainText());
    ui.previewLabel->setPixmap(QPixmap::fromImage(qBinaryImg));
    Mat cvBinaryImg(200, 200, CV_8UC1, (uchar*)qBinaryImg.bits(), qBinaryImg.bytesPerLine());
    hexData = QByteArray::fromHex(image_to_str(cvBinaryImg).toUtf8());
}


void SerialPortDownloader::on_binarizeButton_clicked()
{
    Mat image;
    cvtColor(img_raw, image, COLOR_BGR2GRAY);

    switch (ui.binarizationComboBox->currentIndex())
    {
    case 0:
        threshold(image, image, 128, 255, THRESH_BINARY);
        break;
    case 1:
        adaptiveThreshold(image, image, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 71, 2);
        break;
    case 2:
        image = FloydSteinbergDithering(image);
        break;
    case 3:
        image = JJNDithering(image);
        break;
    default:
        break;
    }
    QString hexString = image_to_str(image);
    hexData = QByteArray::fromHex(hexString.toUtf8());
    show_preview_image(image);
}
