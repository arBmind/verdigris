#include <wobjectcpp.h>
#include <wobjectimpl.h>
#include <QtCore/QObject>
#include <QtCore/QMetaObject>
#include <QtTest/QtTest>

template<class...> struct debug_types;

template<size_t> struct Index {};
template<size_t I> constexpr Index<I> mkIndex = {};
template<size_t I> constexpr size_t indexValue(Index<I>) { return I; }
template<class IV> constexpr size_t index_value = indexValue(IV{});

class tst_CppApi : public QObject
{
    W_OBJECT(tst_CppApi)

#if !defined(Q_CC_GNU) || defined(Q_CC_CLANG) // GCC does not resolve the signals

private slots:
    void firstTest();
    W_SLOT(firstTest, W_Access::Private)
    void notifyTest();
    W_SLOT(notifyTest, W_Access::Private)

private:
    QString m_name{};
public:
    int m_age{};

public:
    QString getName() const { return m_name; }

    //void baseNotify() W_SIGNAL(baseNotify)

    template<size_t I>
    void notifyPropertyChanged() {
        W_CPP_SIGNAL_IMPL(decltype (&tst_CppApi::notifyPropertyChanged<I>), mySignals, I, 0)
    }

    static constexpr auto mySignalsLambda = [](auto i) {
        constexpr auto I = index_value<decltype (i)>;
        using namespace w_cpp;
        if constexpr (I == 0) {
            return makeSignalBuilder(viewLiteral("nameChanged"), &tst_CppApi::notifyPropertyChanged<0>).build();
        }
        else if constexpr (I == 1) {
            return makeSignalBuilder(viewLiteral("ageChanged"), &tst_CppApi::notifyPropertyChanged<1>).build();
        }
    };
    template<size_t I>
    static constexpr auto mySignals() -> decltype (mySignalsLambda(mkIndex<I>)) {
        return mySignalsLambda(mkIndex<I>);
    }
    W_CPP_SIGNAL(mySignals)

private:
    // W_PROPERTY(QString, name2 READ getName NOTIFY notifyPropertyChanged<0>)

    static constexpr auto myPropertiesLambda = [](auto i) {
        constexpr auto I = index_value<decltype (i)>;
        using namespace w_cpp;
        if constexpr (I == 0) {
            return makeProperty<QString>(viewLiteral("name"), viewLiteral("QString"))
                .setGetter(&tst_CppApi::getName)
                .setNotify(&tst_CppApi::notifyPropertyChanged<0>);
        }
        else if constexpr (I == 1) {
            return makeProperty<int>(viewLiteral("age"), viewLiteral("int"))
                .setMember(&tst_CppApi::m_age)
                .setNotify(&tst_CppApi::notifyPropertyChanged<1>);
        }
    };
    template<size_t I>
    static constexpr auto myProperties() -> decltype (myPropertiesLambda(mkIndex<I>)) {
        return myPropertiesLambda(mkIndex<I>);
    }
    W_CPP_PROPERTY(myProperties)
};

void tst_CppApi::firstTest()
{
    const QMetaObject *mo = metaObject();

    QMetaProperty prop = mo->property(mo->indexOfProperty("name"));
    QVERIFY(prop.isValid());
    QVERIFY(!prop.isConstant());

    m_name = "Hello";
    auto v = prop.read(this);
    QCOMPARE(QVariant(m_name), v);

    prop = mo->property(mo->indexOfProperty("age"));
    QVERIFY(prop.isValid());
    QVERIFY(!prop.isConstant());

    prop.write(this, 23);
    QCOMPARE(23, m_age);
}

void tst_CppApi::notifyTest()
{
    bool notified0 = false;
    auto conn0 = connect(this, &tst_CppApi::notifyPropertyChanged<0>, [&](){
        notified0 = true;
    });
    notifyPropertyChanged<0>();

    QVERIFY(notified0);

    bool notified1 = false;
    auto conn1 = connect(this, &tst_CppApi::notifyPropertyChanged<1>, [&](){
        notified1 = true;
    });
    setProperty("age", 100);
    QVERIFY(notified1);

    disconnect(conn0);
    disconnect(conn1);
}

#else // Q_CC_GNU
};

#endif

W_OBJECT_IMPL(tst_CppApi)

QTEST_MAIN(tst_CppApi)