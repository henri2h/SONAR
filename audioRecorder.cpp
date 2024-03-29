/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "audioRecorder.h"

#include <stdlib.h>
#include <math.h>

#include <QDateTime>
#include <QDebug>
#include <QPainter>
#include <QVBoxLayout>
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <qendian.h>
#include <QBuffer>
#include <QTimer>

AudioInfo::AudioInfo(const QAudioFormat &format)
    : m_format(format)
{
    switch (m_format.sampleSize())
    {
    case 8:
        switch (m_format.sampleType())
        {
        case QAudioFormat::UnSignedInt:
            m_maxAmplitude = 255;
            break;
        case QAudioFormat::SignedInt:
            m_maxAmplitude = 127;
            break;
        default:
            break;
        }
        break;
    case 16:
        switch (m_format.sampleType())
        {
        case QAudioFormat::UnSignedInt:
            m_maxAmplitude = 65535;
            break;
        case QAudioFormat::SignedInt:
            m_maxAmplitude = 32767;
            break;
        default:
            break;
        }
        break;

    case 32:
        switch (m_format.sampleType())
        {
        case QAudioFormat::UnSignedInt:
            m_maxAmplitude = 0xffffffff;
            break;
        case QAudioFormat::SignedInt:
            m_maxAmplitude = 0x7fffffff;
            break;
        case QAudioFormat::Float:
            m_maxAmplitude = 0x7fffffff; // Kind of
        default:
            break;
        }
        break;

    default:
        qDebug() << "default";
        break;
    }
}

void AudioInfo::start()
{
    open(QIODevice::WriteOnly);
}

void AudioInfo::stop()
{
    close();
}

qint64 AudioInfo::readData(char *data, qint64 maxlen)
{
    Q_UNUSED(data)
    Q_UNUSED(maxlen)

    return 0;
}

qint64 AudioInfo::writeData(const char *data, qint64 len)
{
    if (m_maxAmplitude)
    {
        Q_ASSERT(m_format.sampleSize() % 8 == 0);
        const int channelBytes = m_format.sampleSize() / 8;
        const int sampleBytes = m_format.channelCount() * channelBytes;
        Q_ASSERT(len % sampleBytes == 0);
        const int numSamples = len / sampleBytes;

        const unsigned char *ptr = reinterpret_cast<const unsigned char *>(data);

        for (int i = 0; i < numSamples; ++i)
        {
            for (int j = 0; j < m_format.channelCount(); ++j)
            {
                qreal value = 0;

                if (m_format.sampleSize() == 8 && m_format.sampleType() == QAudioFormat::UnSignedInt)
                {
                    value = *reinterpret_cast<const quint8 *>(ptr);
                }
                else if (m_format.sampleSize() == 8 && m_format.sampleType() == QAudioFormat::SignedInt)
                {
                    value = *reinterpret_cast<const qint8 *>(ptr);
                }
                else if (m_format.sampleSize() == 16 && m_format.sampleType() == QAudioFormat::UnSignedInt)
                {
                    if (m_format.byteOrder() == QAudioFormat::LittleEndian)
                        value = qFromLittleEndian<quint16>(ptr);
                    else
                        value = qFromBigEndian<quint16>(ptr);
                }
                else if (m_format.sampleSize() == 16 && m_format.sampleType() == QAudioFormat::SignedInt)
                {
                    if (m_format.byteOrder() == QAudioFormat::LittleEndian)
                        value = qFromLittleEndian<qint16>(ptr);
                    else
                        value = qFromBigEndian<qint16>(ptr);
                }
                else if (m_format.sampleSize() == 32 && m_format.sampleType() == QAudioFormat::UnSignedInt)
                {
                    if (m_format.byteOrder() == QAudioFormat::LittleEndian)
                        value = qFromLittleEndian<quint32>(ptr);
                    else
                        value = qFromBigEndian<quint32>(ptr);
                }
                else if (m_format.sampleSize() == 32 && m_format.sampleType() == QAudioFormat::SignedInt)
                {
                    if (m_format.byteOrder() == QAudioFormat::LittleEndian)
                        value = qFromLittleEndian<qint32>(ptr);
                    else
                        value = qFromBigEndian<qint32>(ptr);
                }
                else if (m_format.sampleSize() == 32 && m_format.sampleType() == QAudioFormat::Float)
                {
                    value = *reinterpret_cast<const float *>(ptr) * 0x7fffffff; // assumes 0-1.0
                }

                float q = value * 1.0 / m_maxAmplitude;

                // setup saving
                if (samplesToRecords > 0)
                {
                    soundData.append(q);
                    samplesToRecords--;
                }

                ptr += channelBytes;
            }
        }
    }

    emit update();
    return len;
}

bool AudioInfo::resetData()
{
    soundData.clear();
    return true;
}

void AudioInfo::startRecording(int samples)
{ // should check if we can record
    soundData.clear();
    samplesToRecords = samples;
}

bool AudioInfo::isRecording()
{
    if (samplesToRecords > 0)
        return true;
    else
        return false;
}

AudioRecorder::AudioRecorder()
{
    //    initializeAudio(QAudioDeviceInfo::defaultInputDevice());
}

void AudioRecorder::initializeAudio(QAudioFormat format)
{
    QAudioDeviceInfo deviceInfo = QAudioDeviceInfo::defaultInputDevice();

    if (!deviceInfo.isFormatSupported(format))
    {
        qWarning() << "Default format not supported - trying to use nearest";
        format = deviceInfo.nearestFormat(format);
    }
    m_format = format;

    m_audioInfo.reset(new AudioInfo(format));
    m_audioInput.reset(new QAudioInput(deviceInfo, format));

    m_audioInfo->start();
    m_audioInput->start(m_audioInfo.data());
    // to set the device to be constantly recording
    enabled = true;
}

void AudioRecorder::Record(int time)
{ // time in ms
    int samples = m_format.sampleRate() * time / 1000; // get number of sample to record

    if (m_audioInfo->isRecording() == false)
    {
        // reset data and start recording samples samples
        m_audioInfo->startRecording(samples);
    }
    else
    {
        qWarning() << "Already recording";
    }
}

void AudioRecorder::StopRecording()
{
    enabled = false;
    m_audioInput->stop();
    m_audioInfo->stop();
    m_audioInput->disconnect(this);
    QList<float> sd = m_audioInfo->soundData;
    qDebug() << "Final length : " << sd.length() / 48000;
}

QList<float> AudioRecorder::getRecording()
{
    while (m_audioInfo->isRecording())
    {
        qDebug() << "Wait";
        return QList<float>();
    }
    qDebug() << "buffer length : " << m_audioInfo->soundData.length();
    return m_audioInfo->soundData;
}
