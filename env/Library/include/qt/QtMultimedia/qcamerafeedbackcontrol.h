/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/



#ifndef QCAMERAFEEDBACKCONTROL_H
#define QCAMERAFEEDBACKCONTROL_H

#include <QtMultimedia/qmediacontrol.h>
#include <QtMultimedia/qmediaobject.h>

#include <QtMultimedia/qcamera.h>
#include <QtMultimedia/qmediaenumdebug.h>

QT_BEGIN_NAMESPACE

// Required for QDoc workaround
class QString;

class Q_MULTIMEDIA_EXPORT QCameraFeedbackControl : public QMediaControl
{
    Q_OBJECT

public:
    enum EventType {
        ViewfinderStarted = 1,
        ViewfinderStopped,
        ImageCaptured,
        ImageSaved,
        ImageError,
        RecordingStarted,
        RecordingInProgress,
        RecordingStopped,
        AutoFocusInProgress,
        AutoFocusLocked,
        AutoFocusFailed
    };

    ~QCameraFeedbackControl();

    virtual bool isEventFeedbackLocked(EventType) const = 0;

    virtual bool isEventFeedbackEnabled(EventType) const = 0;

    virtual bool setEventFeedbackEnabled(EventType, bool) = 0;
    virtual void resetEventFeedback(EventType) = 0;

    virtual bool setEventFeedbackSound(EventType, const QString &filePath) = 0;

protected:
    explicit QCameraFeedbackControl(QObject *parent = Q_NULLPTR);
};

#define QCameraFeedbackControl_iid "org.qt-project.qt.camerafeedbackcontrol/5.0"
Q_MEDIA_DECLARE_CONTROL(QCameraFeedbackControl, QCameraFeedbackControl_iid)

QT_END_NAMESPACE


#endif // QCAMERAFEEDBACKCONTROL_H
