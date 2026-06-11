cmake_minimum_required(VERSION 3.19)
project(Vaultea LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Svg)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED libsodium)

include(FetchContent)

# Abseil
set(ABSL_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(ABSL_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    absl
    GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
    GIT_TAG 20260107.1
)

FetchContent_MakeAvailable(absl)

# LZ4
set(LZ4_BUILD_CLI OFF CACHE BOOL "" FORCE)
set(LZ4_BUILD_LEGACY_LZ4C OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    lz4
    GIT_REPOSITORY https://github.com/lz4/lz4.git
    GIT_TAG v1.10.0
)

FetchContent_MakeAvailable(lz4)

qt_standard_project_setup()

set(CMAKE_AUTORCC ON)

qt_add_executable(${PROJECT_NAME}
    WIN32
    MACOSX_BUNDLE

    # RESOURCES
    src/resources.qrc

    # UI
    src/ui/main.cpp

    src/ui/controllers/vaultcontroller.cpp
    src/ui/controllers/vaultcontroller.h

    src/ui/pages/createvault/createvaultpage.cpp
    src/ui/pages/createvault/createvaultpage.h

    src/ui/pages/entryeditor/entryeditorpage.cpp
    src/ui/pages/entryeditor/entryeditorpage.h
    src/ui/pages/entryeditor/entryeditorpage.ui

    src/ui/pages/folderview/editfolderdialog.cpp
    src/ui/pages/folderview/editfolderdialog.h
    src/ui/pages/folderview/editfolderdialog.ui
    src/ui/pages/folderview/folderviewpage.cpp
    src/ui/pages/folderview/folderviewpage.h
    src/ui/pages/folderview/folderviewpage.ui

    src/ui/pages/openvault/openvaultpage.cpp
    src/ui/pages/openvault/openvaultpage.h

    src/ui/pages/vault/vaultpage.cpp
    src/ui/pages/vault/vaultpage.h
    src/ui/pages/vault/vaultpage.ui

    src/ui/pages/welcome/welcomepage.cpp
    src/ui/pages/welcome/welcomepage.h
    src/ui/pages/welcome/welcomepage.ui

    src/ui/theme/thememanager.cpp
    src/ui/theme/thememanager.h

    src/ui/widgets/passwordlineedit.cpp
    src/ui/widgets/passwordlineedit.h

    src/ui/windows/mainwindow.cpp
    src/ui/windows/mainwindow.h
    src/ui/windows/mainwindow.ui

    # CORE
    src/core/VaultEngine.cpp
    src/core/VaultEngine.h
    src/core/VaultLoader.cpp
    src/core/VaultLoader.h

    # CRYPTO
    src/crypto/Constants.h
    src/crypto/CryptoService.cpp
    src/crypto/CryptoService.h

    # INDEX
    src/index/BTreeIndex.cpp
    src/index/BTreeIndex.h
    src/index/Config.h
    src/index/IIndex.h

    # MANAGERS
    src/managers/Constants.h
    src/managers/DataManager.cpp
    src/managers/DataManager.h
    src/managers/IndexManager.cpp
    src/managers/IndexManager.h
    src/managers/SlottedManager.cpp
    src/managers/SlottedManager.h

    # MODELS
    src/models/BaseVaultObject.h
    src/models/ProcessedBlob.h
    src/models/Folder.h
    src/models/IndexEntry.h
    src/models/RecordRef.h
    src/models/VaultEntry.h

    # PIPELINE
    src/pipeline/Codec.h
    src/pipeline/DefaultCodec.cpp
    src/pipeline/DefaultCodec.h
    src/pipeline/ICompressor.h
    src/pipeline/LZ4Compressor.cpp
    src/pipeline/LZ4Compressor.h
    src/pipeline/Serialization.cpp
    src/pipeline/Serialization.h

    # STORAGE
    src/storage/Constants.h
    src/storage/StorageEngine.cpp
    src/storage/StorageEngine.h
    src/storage/UUID.cpp
    src/storage/UUID.h
    src/storage/VaultHeader.h
    src/storage/VaultPreamble.h

    src/storage/page/Constants.h
    src/storage/page/Page.h
    src/storage/page/PageAllocator.cpp
    src/storage/page/PageAllocator.h
    src/storage/page/PageLayout.h
    src/storage/page/Pager.cpp
    src/storage/page/Pager.h
    src/storage/page/SlottedPageHandler.cpp
    src/storage/page/SlottedPageHandler.h

    # UTIL
    src/util/Helpers.h
)


target_include_directories(${PROJECT_NAME}
    PRIVATE
        src
        ${SODIUM_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Svg
        ${SODIUM_LIBRARIES}
        absl::base
        absl::strings
        lz4
)

install(TARGETS ${PROJECT_NAME}
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET ${PROJECT_NAME}
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})
