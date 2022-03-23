TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
    Battery \
    BubbleWindow \
    Chart \
    Clock \
    DashBoard \
    DragDrop \
    FlowLayout \
    GridViewModel \
    HttpClient \
    LogAsynchronous \
    MulClient \
    MulServer \
    NavigationProgressBar \
    ProgressArc \
    ProgressBar \
    ReactorServer \
    SimpleUdp \
    SlipButton \
    SqlTabview \
    TabViewModel \
    Thread \
    TreeViewModel

DISTFILES += \
    $$files(Scripts/*.py)\
    LICENSE \
    README.md
