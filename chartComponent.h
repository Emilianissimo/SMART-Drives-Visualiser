#ifndef CHARTCOMPONENT_H
#define CHARTCOMPONENT_H

#include <QtCharts/QChartView>

class ChartComponent
{
public:
    QChartView* Generate(QString HDDName, QString label, int value);
};

#endif // CHARTCOMPONENT_H
