/****************************************************************************
**
** Copyright (C) 2017 Ford Motor Company
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtRemoteObjects module of the Qt Toolkit.
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

#ifndef QREMOTEOBJECTSOURCE_P_H
#define QREMOTEOBJECTSOURCE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QObject>
#include <QMetaObject>
#include <QMetaProperty>
#include <QVector>
#include "qremoteobjectsource.h"
#include "qremoteobjectpacket_p.h"

QT_BEGIN_NAMESPACE

class QRemoteObjectSourceIo;
class ServerIoDevice;

class QRemoteObjectSource : public QObject
{
public:
    explicit QRemoteObjectSource(QObject *object, const SourceApiMap *,
                                 QObject *adapter, QRemoteObjectSourceIo *sourceIo);

    ~QRemoteObjectSource();

    int qt_metacall(QMetaObject::Call call, int methodId, void **a);
    QVector<ServerIoDevice*> listeners;
    QObject *m_object, *m_adapter;
    const SourceApiMap * const m_api;
    QRemoteObjectSourceIo *m_sourceIo;
    QRemoteObjectPackets::DataStreamPacket m_packet;
    QVariantList m_marshalledArgs;
    bool hasAdapter() const { return m_adapter; }

    QVariantList* marshalArgs(int index, void **a);
    void handleMetaCall(int index, QMetaObject::Call call, void **a);
    void addListener(ServerIoDevice *io, bool dynamic = false);
    int removeListener(ServerIoDevice *io, bool shouldSendRemove = false);
    bool invoke(QMetaObject::Call c, bool forAdapter, int index, const QVariantList& args, QVariant* returnValue = nullptr);
    QByteArray m_objectChecksum;
    static const int qobjectPropertyOffset;
    static const int qobjectMethodOffset;
};

class DynamicApiMap : public SourceApiMap
{
public:
    DynamicApiMap(const QMetaObject *metaObject, const QString &name, const QString &typeName);
    ~DynamicApiMap() {}
    QString name() const override { return m_name; }
    QString typeName() const override { return m_typeName; }
    int propertyCount() const override { return m_properties.size(); }
    int signalCount() const override { return m_signals.size(); }
    int methodCount() const override { return m_methods.size(); }
    int sourcePropertyIndex(int index) const override
    {
        if (index < 0 || index >= propertyCount())
            return -1;
        return m_properties.at(index);
    }
    int sourceSignalIndex(int index) const override
    {
        if (index < 0 || index >= signalCount())
            return -1;
        return m_signals.at(index);
    }
    int sourceMethodIndex(int index) const override
    {
        if (index < 0 || index >= methodCount())
            return -1;
        return m_methods.at(index);
    }
    int signalParameterCount(int index) const override { return parameterCount(m_signals.at(index)); }
    int signalParameterType(int sigIndex, int paramIndex) const override { return parameterType(m_signals.at(sigIndex), paramIndex); }
    const QByteArray signalSignature(int index) const override { return signature(m_signals.at(index)); }
    int methodParameterCount(int index) const override { return parameterCount(m_methods.at(index)); }
    int methodParameterType(int methodIndex, int paramIndex) const override { return parameterType(m_methods.at(methodIndex), paramIndex); }
    const QByteArray methodSignature(int index) const override { return signature(m_methods.at(index)); }
    QMetaMethod::MethodType methodType(int index) const override;
    const QByteArray typeName(int index) const override;
    int propertyIndexFromSignal(int index) const override
    {
        if (index >= 0 && index < m_propertyAssociatedWithSignal.size())
            return m_properties.at(m_propertyAssociatedWithSignal.at(index));
        return -1;
    }
    int propertyRawIndexFromSignal(int index) const override
    {
        if (index >= 0 && index < m_propertyAssociatedWithSignal.size())
            return m_propertyAssociatedWithSignal.at(index);
        return -1;
    }
    QByteArray objectSignature() const override { return m_objectSignature; }

    bool isDynamic() const override { return true; }
private:
    int parameterCount(int objectIndex) const;
    int parameterType(int objectIndex, int paramIndex) const;
    const QByteArray signature(int objectIndex) const;
    inline void checkCache(int objectIndex) const
    {
        if (objectIndex != m_cachedMetamethodIndex) {
            m_cachedMetamethodIndex = objectIndex;
            m_cachedMetamethod = m_metaObject->method(objectIndex);
        }
    }

    QString m_name;
    QString m_typeName;
    QVector<int> m_properties;
    QVector<int> m_signals;
    QVector<int> m_methods;
    QVector<int> m_propertyAssociatedWithSignal;
    const QMetaObject *m_metaObject;
    mutable QMetaMethod m_cachedMetamethod;
    mutable int m_cachedMetamethodIndex;
    QByteArray m_objectSignature;
};

QT_END_NAMESPACE

#endif
