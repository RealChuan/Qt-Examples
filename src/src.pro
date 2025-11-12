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
    CircularProgress \
    Clock \
    DashBoard \
    GridViewModel \
    HttpClient \
    LifecycleCallback \
    LoadingIndicator \
    NavigationProgressBar \
    PasswordLineEdit \
    ProgressBar \
    ReactorServer \
    SqliteWAL \
    SwitchButton

win32 {
    SUBDIRS += WindowsIntegration
}
