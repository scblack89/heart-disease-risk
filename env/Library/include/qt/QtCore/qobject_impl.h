/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
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

#ifndef Q_QDOC

#ifndef QOBJECT_H
#error Do not include qobject_impl.h directly
#endif

#if 0
#pragma qt_sync_skip_header_check
#pragma qt_sync_stop_processing
#endif

QT_BEGIN_NAMESPACE


namespace QtPrivate {
    /*
        Logic to statically generate the array of qMetaTypeId
        ConnectionTypes<FunctionPointer<Signal>::Arguments>::types() returns an array
        of int that is suitable for the types arguments of the connection functions.

        The array only exist of all the types are declared as a metatype
        (detected using the TypesAreDeclaredMetaType helper struct)
        If one of the type is not declared, the function return 0 and the signal
        cannot be used in queued connection.
    */
    template <typename ArgList> struct TypesAreDeclaredMetaType { enum { Value = false }; };
    template <> struct TypesAreDeclaredMetaType<List<>> { enum { Value = true }; };
    template <typename Arg, typename... Tail> struct TypesAreDeclaredMetaType<List<Arg, Tail...> >
    { enum { Value = QMetaTypeId2<Arg>::Defined && TypesAreDeclaredMetaType<List<Tail...>>::Value }; };

    template <typename ArgList, bool Declared = TypesAreDeclaredMetaType<ArgList>::Value > struct ConnectionTypes
    { static const int *types() { return Q_NULLPTR; } };
    template <> struct ConnectionTypes<List<>, true>
    { static const int *types() { return Q_NULLPTR; } };
    template <typename... Args> struct ConnectionTypes<List<Args...>, true>
    { static const int *types() { static const int t[sizeof...(Args) + 1] = { (QtPrivate::QMetaTypeIdHelper<Args>::qt_metatype_id())..., 0 }; return t; } };

    // internal base class (interface) containing functions required to call a slot managed by a pointer to function.
    class QSlotObjectBase {
        QAtomicInt m_ref;
        // don't use virtual functions here; we don't want the
        // compiler to create tons of per-polymorphic-class stuff that
        // we'll never need. We just use one function pointer.
        typedef void (*ImplFn)(int which, QSlotObjectBase* this_, QObject *receiver, void **args, bool *ret);
        const ImplFn m_impl;
    protected:
        enum Operation {
            Destroy,
            Call,
            Compare,

            NumOperations
        };
    public:
        explicit QSlotObjectBase(ImplFn fn) : m_ref(1), m_impl(fn) {}

        inline int ref() Q_DECL_NOTHROW { return m_ref.ref(); }
        inline void destroyIfLastRef() Q_DECL_NOTHROW
        { if (!m_ref.deref()) m_impl(Destroy, this, Q_NULLPTR, Q_NULLPTR, Q_NULLPTR); }

        inline bool compare(void **a) { bool ret = false; m_impl(Compare, this, Q_NULLPTR, a, &ret); return ret; }
        inline void call(QObject *r, void **a)  { m_impl(Call,    this, r, a, Q_NULLPTR); }
    protected:
        ~QSlotObjectBase() {}
    private:
        Q_DISABLE_COPY(QSlotObjectBase)
    };
    // implementation of QSlotObjectBase for which the slot is a pointer to member function of a QObject
    // Args and R are the List of arguments and the returntype of the signal to which the slot is connected.
    template<typename Func, typename Args, typename R> class QSlotObject : public QSlotObjectBase
    {
        typedef QtPrivate::FunctionPointer<Func> FuncType;
        Func function;
        static void impl(int which, QSlotObjectBase *this_, QObject *r, void **a, bool *ret)
        {
            switch (which) {
            case Destroy:
                delete static_cast<QSlotObject*>(this_);
                break;
            case Call:
                FuncType::template call<Args, R>(static_cast<QSlotObject*>(this_)->function, static_cast<typename FuncType::Object *>(r), a);
                break;
            case Compare:
                *ret = *reinterpret_cast<Func *>(a) == static_cast<QSlotObject*>(this_)->function;
                break;
            case NumOperations: ;
            }
        }
    public:
        explicit QSlotObject(Func f) : QSlotObjectBase(&impl), function(f) {}
    };
    // implementation of QSlotObjectBase for which the slot is a static function
    // Args and R are the List of arguments and the returntype of the signal to which the slot is connected.
    template<typename Func, typename Args, typename R> class QStaticSlotObject : public QSlotObjectBase
    {
        typedef QtPrivate::FunctionPointer<Func> FuncType;
        Func function;
        static void impl(int which, QSlotObjectBase *this_, QObject *r, void **a, bool *ret)
        {
            switch (which) {
            case Destroy:
                delete static_cast<QStaticSlotObject*>(this_);
                break;
            case Call:
                FuncType::template call<Args, R>(static_cast<QStaticSlotObject*>(this_)->function, r, a);
                break;
            case Compare:   // not implemented
            case NumOperations:
                Q_UNUSED(ret);
            }
        }
    public:
        explicit QStaticSlotObject(Func f) : QSlotObjectBase(&impl), function(f) {}
    };
    // implementation of QSlotObjectBase for which the slot is a functor (or lambda)
    // N is the number of arguments
    // Args and R are the List of arguments and the returntype of the signal to which the slot is connected.
    template<typename Func, int N, typename Args, typename R> class QFunctorSlotObject : public QSlotObjectBase
    {
        typedef QtPrivate::Functor<Func, N> FuncType;
        Func function;
        static void impl(int which, QSlotObjectBase *this_, QObject *r, void **a, bool *ret)
        {
            switch (which) {
            case Destroy:
                delete static_cast<QFunctorSlotObject*>(this_);
                break;
            case Call:
                FuncType::template call<Args, R>(static_cast<QFunctorSlotObject*>(this_)->function, r, a);
                break;
            case Compare: // not implemented
            case NumOperations:
                Q_UNUSED(ret);
            }
        }
    public:
        explicit QFunctorSlotObject(const Func &f) : QSlotObjectBase(&impl), function(f) {}
    };
}


QT_END_NAMESPACE

#endif
