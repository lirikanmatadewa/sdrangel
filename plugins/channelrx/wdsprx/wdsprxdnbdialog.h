///////////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2024 Edouard Griffiths, F4EXB <f4exb06@gmail.com>                   //
//                                                                                   //
// This program is free software; you can redistribute it and/or modify              //
// it under the terms of the GNU General Public License as published by              //
// the Free Software Foundation as version 3 of the License, or                      //
// (at your option) any later version.                                               //
//                                                                                   //
// This program is distributed in the hope that it will be useful,                   //
// but WITHOUT ANY WARRANTY; without even the implied warranty of                    //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                      //
// GNU General Public License V3 for more details.                                   //
//                                                                                   //
// You should have received a copy of the GNU General Public License                 //
// along with this program. If not, see <http://www.gnu.org/licenses/>.              //
///////////////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDE_WDSPRXDNBDIALOG_H
#define INCLUDE_WDSPRXDNBDIALOG_H

#include <QDialog>

#include "export.h"
#include "wdsprxsettings.h"

namespace Ui {
    class WDSPRxDNBDialog;
}

class SDRGUI_API WDSPRxDNBDialog : public QDialog {
    Q_OBJECT
public:
    enum ValueChanged {
        ChangedNB,
        ChangedNB2Mode,
        ChangedNBSlewTime,
        ChangedNBLeadTime,
        ChangedNBLagTime,
        ChangedNBThreshold,
        ChangedNB2SlewTime,
        ChangedNB2LeadTime,
        ChangedNB2LagTime,
        ChangedNB2Threshold,
    };

    explicit WDSPRxDNBDialog(QWidget* parent = nullptr);
    ~WDSPRxDNBDialog();

    void setNBScheme(WDSPRxProfile::WDSPRxNBScheme scheme);
    void setNB2Mode(WDSPRxProfile::WDSPRxNB2Mode mode);
    void setNBSlewTime(double time);
    void setNBLeadTime(double time);
    void setNBLagTime(double time);
    void setNBThreshold(int threshold);
    void setNB2SlewTime(double time);
    void setNB2LeadTime(double time);
    void setNB2LagTime(double time);
    void setNB2Threshold(int threshold);

    WDSPRxProfile::WDSPRxNBScheme getNBScheme() const { return m_nbScheme; }
    WDSPRxProfile::WDSPRxNB2Mode getNB2Mode() const { return m_nb2Mode; }
    double getNBSlewTime() const { return m_nbSlewTime; }
    double getNBLeadTime() const { return m_nbLeadTime; }
    double getNBLagTime() const { return m_nbLagTime; }
    int getNBThreshold() const { return m_nbThreshold; }
    double getNB2SlewTime() const { return m_nb2SlewTime; }
    double getNB2LeadTime() const { return m_nb2LeadTime; }
    double getNB2LagTime() const { return m_nb2LagTime; }
    int getNB2Threshold() const { return m_nb2Threshold; }

signals:
    void valueChanged(int valueChanged);

private:
    Ui::WDSPRxDNBDialog *ui;
    WDSPRxProfile::WDSPRxNBScheme m_nbScheme;
    WDSPRxProfile::WDSPRxNB2Mode m_nb2Mode;
    double m_nbSlewTime;
    double m_nbLeadTime;
    double m_nbLagTime;
    int m_nbThreshold;
    double m_nb2SlewTime;
    double m_nb2LeadTime;
    double m_nb2LagTime;
    int m_nb2Threshold;

private slots:
    void on_nb_currentIndexChanged(int index);
    void on_nb2Mode_currentIndexChanged(int index);
    void on_nbSlewTime_valueChanged(double value);
    void on_nbLeadTime_valueChanged(double value);
    void on_nbLagTime_valueChanged(double value);
    void on_nbThreshold_valueChanged(int value);
    void on_nb2SlewTime_valueChanged(double value);
    void on_nb2LeadTime_valueChanged(double value);
    void on_nb2LagTime_valueChanged(double value);
    void on_nb2Threshold_valueChanged(int value);
};

#endif // INCLUDE_WDSPRXDNBDIALOG_H
