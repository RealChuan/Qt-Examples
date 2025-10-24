#include <QTimer>
#include <QtCore/QCoreApplication>

#include "thread.hpp"

auto main(int argc, char *argv[]) -> int
{
    QCoreApplication a(argc, argv);

    QScopedPointer<Thread1> t1(new Thread1);
    t1->start();

    QScopedPointer<Thread2> t2(new Thread2);
    t2->start();
    QObject::connect(t2.data(), &Thread2::started, t2.data(), &Thread2::onDo);

    QScopedPointer<Thread3> t3(new Thread3);
    t3->dooo();

    runThread4();

    runThread5();

    QScopedPointer<QThread> t6(runThread6());
    t6->start();

    QTimer::singleShot(1000, &a, &QCoreApplication::quit);

    return a.exec();
}
