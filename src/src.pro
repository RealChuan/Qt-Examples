TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
    AutoStartManager \
    Battery \
    Bubble \
#    Chart \
    CheckableTreeItem \
    Clock \
    DashBoard \
    DragDrop \
    FlowLayout \
    GridViewModel \
    HttpClient \
    IconButton \
    ImageCarousel \
    LifecycleCallback \
    LoadingIndicator \
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

win32 {
    SUBDIRS += ShowInMyComputer
}
