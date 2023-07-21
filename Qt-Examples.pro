TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
    Battery \
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
    SqlTabview \
    TableViewModel \
    Thread \
    TreeViewModel \
    Validator

win32{
SUBDIRS += ShowInMyComputer
}

DISTFILES += \
    $$files(Scripts/windows/*.py)\
    $$files(Scripts/windows/*.iss)\
    $$files(Scripts/macos/*.py)\
    $$files(Scripts/macos/*.json)\
    LICENSE \
    README.md
