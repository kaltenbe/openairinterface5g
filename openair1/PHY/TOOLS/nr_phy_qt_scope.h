/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

/* Header file generated by Bo Zhao (Fraunhofer IIS) on Wed. Dec. 08, 2021*/

#ifndef QT_SCOPE_MAINWINDOW_H
#define QT_SCOPE_MAINWINDOW_H

#include <QComboBox>
#include <QMessageBox>

#include <QtCharts>
#include <QChartView>
#include <QLineSeries>

extern "C" {
#include <simple_executable.h>
#include <common/utils/system.h>
#include "common/ran_context.h"
#include <openair1/PHY/defs_gNB.h>
#include "PHY/defs_gNB.h"
#include "PHY/defs_nr_UE.h"
#include "PHY/defs_RU.h"
#include "executables/softmodem-common.h"
#include "phy_scope_interface.h"
#include <openair2/LAYER2/NR_MAC_gNB/mac_proto.h>
#include "PHY/CODING/nrPolar_tools/nr_polar_defs.h"

extern RAN_CONTEXT_t RC;
}

// drop-down list UE
class KPIListSelect : public QComboBox
{
    Q_OBJECT

public:
    explicit KPIListSelect(QWidget *parent = 0);
	~KPIListSelect();

private:

};

// drop-down list gNB
class KPIListSelectgNB : public QComboBox
{
    Q_OBJECT

public:
    explicit KPIListSelectgNB(QWidget *parent = 0);
	~KPIListSelectgNB();

private:

};


// Paint first on a pixmap, then on the widget. Previous paint is erased
class PainterWidget : public QWidget
{
    Q_OBJECT

public:
    PainterWidget(QComboBox *parent, PHY_VARS_NR_UE *ue);
    void makeConnections();
    QPixmap *pix;
    QTimer *timer;
    int chartHight, chartWidth;
    QComboBox *parentWindow;

    QTimer *timerWaterFallTime;


    // for waterfill graphs
    int waterFallh;
    int iteration;
    double *waterFallAvg;
    bool isWaterFallTimeActive;

    extended_kpi_ue extendKPIUE;

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event) override;

public slots:

    // IQ constellations
    void paintPixmap_uePdschIQ();
    void paintPixmap_uePbchIQ();
    void paintPixmap_uePdcchIQ();


    // LLR plots 
    void paintPixmap_uePdschLLR();
    void paintPixmap_uePbchLLR();
    void paintPixmap_uePdcchLLR();

    // Waterfall
    void paintPixmap_ueWaterFallTime();

    void paintPixmap_ueChannelResponse();

private:
	PHY_VARS_NR_UE *ue;
    int indexToPlot;
    int previousIndex;
    int chartBaseHeight;
    int chartBaseWidth;

};


// Paint class for gNB
class PainterWidgetgNB : public QWidget
{
    Q_OBJECT

public:
    PainterWidgetgNB(QComboBox *parent, scopeData_t *p);
    void makeConnections();
    void createPixMap(float *xData, float *yData, int len, QColor MarkerColor, const QString xLabel, const QString yLabel, bool scaleX);

    QPixmap *pix;
    QTimer *timer;
    int chartHight, chartWidth;
    int nb_UEs;

    QComboBox *parentWindow;

    extended_kpi_gNB extendKPIgNB;

    NR_UE_info_t *targetUE;

    QLineSeries *seriesULBLER;
    QLineSeries *seriesULMCS;
    QLineSeries *seriesDLBLER;
    QLineSeries *seriesDLMCS;
    QLineSeries *seriesULThrou;
    QLineSeries *seriesDLThrou;

    float ul_thr_ue, dl_thr_ue;

protected:
    void paintEvent(QPaintEvent *event);

public slots:
    void KPI_PuschIQ();
    void KPI_PuschLLR();
    void KPI_ChannelResponse();
    void KPI_UL_BLER();
    void KPI_UL_MCS();
    void KPI_DL_BLER();
    void KPI_DL_MCS();
    void KPI_UL_Throu();
    void KPI_DL_Throu();

private:
    scopeData_t *p;
    int indexToPlot;
    int previousIndex;
};


#endif // QT_SCOPE_MAINWINDOW_H
