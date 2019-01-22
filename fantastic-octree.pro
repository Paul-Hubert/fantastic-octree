SHADERS = $$files(./src/shaders/*.glsl, true)

spirv.output = ${QMAKE_FILE_NAME}.spv
spirv.commands = glslangValidator -V --target-env vulkan1.1 ${QMAKE_FILE_NAME} -o ${QMAKE_FILE_OUT}
spirv.depends = $$SHADERS
spirv.input = SHADERS
spirv.variable_out = COMPILED_SHADERS
spirv.CONFIG = target_predeps

SOURCES = $$files(src/*.cpp, true)
HEADERS = $$files(src/*.h, true)

# install
target.path = build
target.depends = spirv

DESTDIR = bin #Target file directory
OBJECTS_DIR = build #Intermediate object files directory
MOC_DIR = build #Intermediate moc files directory

QMAKE_CXXFLAGS += -Wclass-memaccess

CONFIG += debug
QMAKE_EXTRA_COMPILERS += spirv

CONFIG += vulkan
CONFIG += exceptions
LIBS += -lvulkan
INCLUDEPATH += $$(VULKAN_SDK)/include
INCLUDEPATH += /home/paulhubert/VulkanSDK/1.1.92.1/x86_64/include
INCLUDEPATH += $$(VULKAN_SDK)/Include
INCLUDEPATH += D:/Qt
