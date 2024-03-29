/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Charts module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "sonar.h"

#include "mainwindow.h"

#include <QtMultimedia/QAudioDeviceInfo>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    const QAudioDeviceInfo inputDevice = QAudioDeviceInfo::defaultInputDevice();
    const QAudioDeviceInfo outputDevice = QAudioDeviceInfo::defaultOutputDevice();

    if (false)
    {
        qDebug() << "Data from the microphone : " << inputDevice.deviceName().toStdString().c_str();

        QList<int> supportedInputSampleRate = inputDevice.supportedSampleRates();

        for (int i = 0; i < supportedInputSampleRate.count(); i++)
        {
            qDebug() << "Supported input sample rate : " << supportedInputSampleRate[i];
        }
        QList<int> supportedOutputSampleRate = outputDevice.supportedSampleRates();

        for (int i = 0; i < supportedOutputSampleRate.count(); i++)
        {
            qDebug() << "Supported output sample rate : " << supportedOutputSampleRate[i];
        }
    }

    if (inputDevice.isNull())
    {
        QMessageBox::warning(nullptr, "audio",
                             "There is no audio input device available.");
        return -1;
    }
    MainWindow mainWindow;
    mainWindow.show();
    return a.exec();
}
