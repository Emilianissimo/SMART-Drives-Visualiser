#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <chartComponent.h>
#include <QtCharts/QChartView>
#include <SMARTReader.h>
#include <iostream>

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    SMARTReader reader;
    vector<map<string, string>> formattedResult = reader.SmartData;

    for (int i = 0; i < formattedResult.size(); i++){
        map<string, string> device = formattedResult[i];
        ChartComponent chartComponent;
        // Define charts
        QString percent = QString::fromStdString(device["remaining_lifetime_percent"]);
        QString name = QString::fromStdString(device["name"]);
        QString hddLabel = QString("Health: %1\%").arg(percent);
        QChartView *chartView = chartComponent.Generate(
            name, hddLabel, stoi(device["remaining_lifetime_percent"])
            );
        ui->mainGrid->addWidget(chartView, 0, i);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::mousePressEvent(QMouseEvent *event){
    current_position = event->globalPosition().toPoint();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event){
    new_position = QPoint(event->globalPosition().toPoint() - current_position);
    move(x() + new_position.x(), y() + new_position.y());
    current_position = event->globalPosition().toPoint();
}

void MainWindow::on_close_clicked()
{
    this->close();
}

