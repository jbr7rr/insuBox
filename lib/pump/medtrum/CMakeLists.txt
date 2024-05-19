
zephyr_library()
zephyr_library_sources(MedtrumDevice.cpp)
zephyr_library_sources(comm/WriteCommandPackets.cpp)
zephyr_library_sources(comm/PumpBleComm.cpp)
zephyr_library_sources(comm/PumpScanner.cpp)
zephyr_library_sources(crypt/Crypt.cpp)
zephyr_library_sources(utils/CrcUtil.cpp)

