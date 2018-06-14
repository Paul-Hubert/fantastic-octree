SOURCES = $$files(*.cpp, true)
HEADERS = $$files(*.h, true)

# install
target.path = build
INSTALLS += target

CONFIG+=debug