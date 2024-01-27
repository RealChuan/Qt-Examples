TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
    Battery \
    BatteryQuick \
    Bootstrap \
    BubbleWindow \
    Chart \
    CheckBoxStandardItem \
    Clock \
    DashBoard \
    DragDrop \
    FlowLayout \
    GridViewModel \
    HttpClient \
    IconButton \
    ImageCarousel \
    LogAsynchronous \
    MulClient \
    MulServer \
    NavigationProgressBar \
    PasswordLineEdit \
    ProgressArc \
    ProgressBar \
    ReactorServer \
    SimpleUdp \
    SlipButton \
    SqliteWAL \
    TableViewModel \
    Thread \
    TreeViewModel \
    Validator

win32{
SUBDIRS += ShowInMyComputer
}

DISTFILES += \
    LICENSE \
    README*
