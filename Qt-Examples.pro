TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
    Battery \
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

DISTFILES += \
    $$files(Scripts/windows/*.py)\
    $$files(Scripts/windows/*.iss)\
    $$files(Scripts/macos/*.py)\
    $$files(Scripts/macos/*.json)\
    LICENSE \
    README.md
