﻿#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <tesseract/baseapi.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QDirIterator>
#include <QImage>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QDebug>
#include <vector>

#include <iostream>

// Array locations when RGB is split.
#define RED    2
#define GREEN  1
#define BLUE   0

#define OCR_WHITELIST "ABCDEFGHIJKLMNOPQRSTUVWXYZ-'()"

const int threshold_value = 128;
const int threshold_fill  = 255;
const int threshold_type  = 0;
using namespace cv;

struct StudentDetails {
   QString forename;
   QString surname;
   QString group;
   bool dubious = false;
};


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/ico/running"));
}


MainWindow::~MainWindow()
{
    delete ui;
}

double skew (Mat& src, int threshold, int percentage)
{
    Size image_size = src.size();
    std::vector<Vec4i> lines;
    HoughLinesP(src, lines, 1, CV_PI/180,threshold, image_size.width / 6, image_size.width / 100 * percentage);
    Mat disp_lines(image_size, CV_8UC1, Scalar(0, 0, 0));
    double angle = 0.;
    unsigned nb_lines = lines.size();
    for (unsigned i = 0; i < nb_lines; ++i)
    {
        line(disp_lines, Point(lines[i][0], lines[i][1]),
            Point(lines[i][2], lines[i][3]), Scalar(255, 0 ,0));
        angle += atan2((double)lines[i][3] - lines[i][1],
            (double)lines[i][2] - lines[i][0]);
    }
    angle /= nb_lines; // mean angle, in radians.

    // Return angle in degrees.
    return angle * 180 / CV_PI;
}

void rotate(Mat& img, float angle)
{
    int len = max(img.cols, img.rows);
    Point2f pt(len/2.f, len/2.f);
    Mat r = getRotationMatrix2D(pt, angle, 1.0);

    warpAffine(img, img, r, Size(len, len));
}

StudentDetails doOCR (Mat& src)
{
    // Setup the Tesseract
    tesseract::TessBaseAPI t;
    t.Init(NULL, "eng");
    t.SetVariable("tessedit_char_whitelist", OCR_WHITELIST);
    t.SetVariable("load_system_dawg", "0");
    t.SetVariable("load_freq_dawg", "0");
    t.SetVariable("textord_min_linesize", "2.5");
    t.SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);

    // Load the image into Tesseract
    t.SetImage((uchar*)src.data, src.cols, src.rows, 1, src.cols);

    // Do the image processing, get the OCR'd text out of it
    char * ocr = t.GetUTF8Text();
    QString capturedText = QString::fromUtf8((ocr));

    // Split the captured text by the New Line character
    QRegExp rx("(\\n)");
    QStringList details = capturedText.split(rx);

    // Load all the text into the appropriate variables.
    StudentDetails student;
    if (details.count() > 2) {
        // Regex select the group name/letter.
        // TODO: This is just one letter. Expand to include numbers? Or document this somewhere.
        rx.setPattern(".*GROUP *([A-Z])");
        rx.indexIn(details[2]);

        // Copy details across.
        student.forename = details[1].trimmed();
        student.surname = details[0].trimmed();
        student.group = rx.cap(1);

        // If we can't find anything in one or more of the OCRd fields
        // then error out and mark the image as dubious.
        if (details[0] == "" || details[1] == "" || rx.cap(1) == "") {
            student.dubious = true;
        }

    } else {
        student.dubious = true;
    }
    return student;
}

StudentDetails processImage (QString file, int houghThreshold, int houghPercentage)
{

    Mat inputImage = cv::imread(file.toStdString());
    StudentDetails capturedStudent;

        Mat temp;

        // Split into component channels, assumes an RGB input image.
        Mat channel[5];
        split(inputImage, channel);

        // Subtract Red channel from Green channel, binary threshold
        // and load into contourMask.
        Mat contourMask;
        threshold(channel[RED] - channel[GREEN],
                  contourMask,
                  threshold_value,
                  threshold_fill,
                  threshold_type);
        // Process the image and find the contours, load that into a holding variable, "contours".
        std::vector<std::vector<Point> > contours;
        findContours(contourMask, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

        // Loop through all the found contours, assess the size of each.
        double max_area = 0;
        int max_idx = 0;
        if (contours.size() > 0) {
        for (unsigned int i = 0; i < contours.size(); i++)
            {
                double area = contourArea(contours[i]);
                max_idx  = area > max_area ? i : max_idx;
                max_area = area > max_area ? area : max_area;
            }
        } else {
            // did you use red sheets?
            QMessageBox msgErrorBox;
            msgErrorBox.setWindowTitle("No red contours found");
            msgErrorBox.setText("No significant red areas have been detected in this image.\n\nDid you use red name sheets?\n\nImage format is assumed to be RGB.");
            msgErrorBox.setIcon(QMessageBox::Critical);
            msgErrorBox.exec();
            capturedStudent.dubious = true;
            return capturedStudent;
        }

        // Create a rectangle to crop the image to using the largest contour
        // found in the procedure above.
        Rect boundingBox;
        boundingBox = boundingRect(contours[max_idx]);

        // Fill the mask with black in preparation
        contourMask.setTo(Scalar(0));

        // Draw the largest contour on the page in white, then apply the mask
        // to the red channel (where we will get out information from) and
        // threshold that data to obtain clean text data.
        drawContours(contourMask, contours, max_idx, Scalar(255), -1);

        threshold(contourMask - channel[RED],
            channel[4],
            threshold_value,
            threshold_fill,
            threshold_type);

        // Crop the image to the largest contour found above.
        Mat cropped = channel[4](boundingBox);

        // Detect the skew
        float angle = skew(cropped, houghThreshold, houghPercentage);

        angle = angle;

        // Straighten the image to the appropriate amount
        rotate (cropped, angle);
        capturedStudent = doOCR(cropped);
        qDebug() << capturedStudent.forename << capturedStudent.surname << capturedStudent.group;
        return capturedStudent;

}

void MainWindow::on_pushButton_clicked()
{
    StudentDetails student;

    // Load a list of the files in the "input" folder into a list to iterate over.
    QDirIterator it(ui->lineImageURI->text(), QStringList() << "*.jpg", QDir::Files);

    // If the segregate the uncertain images box is checked then check if the "uncertain" output
    // folder exists. Create it if it doesn't.
    if (ui->checkOcrSegregate->checkState() == Qt::Checked) {
        QDir dir(ui->lineOutputURI->text() + "/uncertain/");
        if (!dir.exists()) {
            dir.mkpath(".");
        }
    }

    // Iterate over the list of input files.
    while (it.hasNext()) {

        it.next();
        qDebug() << "Processing:\t" << it.fileInfo().absoluteFilePath();

        // Do the image processing, load the returned data into the student variable
        student = processImage(it.fileInfo().absoluteFilePath(),ui->spinHoughThreshold->value(), ui->spinHoughGapPercentage->value());

        QString outputFile;

        // If the dubious variable is flagged and we are segregating them and not just ignoring them
        if (student.dubious && ui->checkOcrSegregate->checkState() == Qt::Checked) {

            outputFile = QString (ui->lineOutputURI->text() + "/uncertain/" + it.fileName());
            qDebug() << "OCR Failure: " << outputFile;
            QFile::copy(it.fileInfo().absoluteFilePath(), outputFile);

        } else {

            // The file could be read, and the OCR returned some data.
            // What is the selected naming format? Construct the output filename.
            switch (ui->comboNamingFormat->currentIndex()) {

                case 0: // SURNAME, FORENAME (GROUP X)
                    outputFile = QString (ui->lineOutputURI->text() + "/" + student.surname.toUpper() + ", " + student.forename.toUpper() + " (GROUP " + student.group.toUpper() + ").JPG");
                    break;

                case 1: // SURNAME, FORENAME
                    outputFile = QString (ui->lineOutputURI->text() + "/" + student.surname.toUpper() + ", " + student.forename.toUpper() + ".JPG");
                    break;

                case 2: // GROUP X/SURNAME, FORENAME
                    // Check if the output group directory exists,
                    // and create it if it doesn't.
                    QDir dir(ui->lineOutputURI->text() + "/GROUP " + student.group.toUpper());
                    if (!dir.exists()) {
                        dir.mkpath(".");
                    }
                    outputFile = QString (ui->lineOutputURI->text() + "/GROUP " + student.group.toUpper() + "/" + student.surname.toUpper() + ", " + student.forename.toUpper() + ".JPG");
                    break;

            }

            // Actually copy the file across here.
            qDebug() << "OCR Success: " << outputFile;
            QFile::copy(it.fileInfo().absoluteFilePath(), outputFile);
        }

    }
}

void MainWindow::on_pushImageSelector_clicked()
{
    QString filename = QFileDialog::getExistingDirectory(this, tr("Images to process"), QDir::toNativeSeparators(QDir::homePath()));
    ui->lineImageURI->setText(filename);
}

void MainWindow::on_pushImageSelector_2_clicked()
{
    QString filename = QFileDialog::getExistingDirectory(this, tr("Output folder"), QDir::toNativeSeparators(QDir::homePath()));
    ui->lineOutputURI->setText(filename);
}
