cmake_minimum_required(VERSION 3.19)
project(Vaultea LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED libsodium)

include(FetchContent)

FetchContent_Declare(
    absl
    GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
    GIT_TAG 20260107.1
)

FetchContent_MakeAvailable(absl)

qt_standard_project_setup()

qt_add_executable(${PROJECT_NAME}
    WIN32
    MACOSX_BUNDLE

    # APP
    src/app/main.cpp

    # UI
    src/ui/mainwindow.cpp
    src/ui/mainwindow.h
    src/ui/mainwindow.ui

    # CORE
    src/core/VaultEngine.cpp
    src/core/VaultEngine.h

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
    src/models/EncryptedBlob.h
    src/models/EntryTypes.h
    src/models/Folder.h
    src/models/IndexEntry.h
    src/models/RecordRef.h
    src/models/VaultEntry.h

    # PIPELINE
    src/pipeline/Codec.h
    src/pipeline/Compression.cpp
    src/pipeline/Compression.h
    src/pipeline/DefaultCodec.cpp
    src/pipeline/DefaultCodec.h
    src/pipeline/Serialization.cpp
    src/pipeline/Serialization.h

    # STORAGE
    src/storage/Constants.h
    src/storage/StorageEngine.cpp
    src/storage/StorageEngine.h
    src/storage/UUID.cpp
    src/storage/UUID.h
    src/storage/VaultHeader.h
    src/storage/VaultLayout.h
    src/storage/VaultPreamble.h
    src/storage/VaultTypes.h

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
        ${SODIUM_LIBRARIES}
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
