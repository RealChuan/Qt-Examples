TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
    AutoStartManager \
    Battery \
    Bubble \
    ButtonIconStateManager \
    Carousel3DView \
#    Chart \
    CheckableTreeItem \
    Clock \
    DashBoard \
    GridViewModel \
    HttpClient \
    LifecycleCallback \
    LoadingIndicator \
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
