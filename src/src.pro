TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
    AutoStartManager \
    Battery \
    Bubble \
    ButtonIconStateManager \
#    Chart \
    CheckableTreeItem \
    Clock \
    DashBoard \
    GridViewModel \
    HttpClient \
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
