#include "chartComponent.h"
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>


QChartView* ChartComponent::Generate(
    QString HDDName,
    QString label,
    int value
) {
    // DonutBreakdownChart donutChart = new DonutBreakdownChart;
    QPieSeries *series = new QPieSeries();
    series->append(label, value);
    series->append("", 100 - value);
    series->setHoleSize(0.5);

    QPieSlice *slicePositivePart = series->slices().at(0);
    slicePositivePart->setLabelVisible();
    slicePositivePart->setLabelBrush(QBrush(QColor("white")));
    slicePositivePart->setBrush(QBrush(QColor("#00a88e")));
    slicePositivePart->setBorderWidth(0);

    QPieSlice *sliceNegativePart = series->slices().at(1);
    sliceNegativePart->setBorderWidth(0);
    sliceNegativePart->setBrush(QBrush(QColor("#003129")));

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(HDDName);
    chart->setTitleBrush(QBrush(QColor("white")));
    chart->legend()->hide();
    chart->setBackgroundBrush(QBrush(QColor("transparent")));

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setBackgroundBrush(QBrush(QColor("#094d42")));
    return chartView;
}
