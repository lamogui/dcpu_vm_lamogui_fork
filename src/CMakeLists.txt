#include(GL)
include(SFML)

set(dcpu_SRCS
    ./dcpu/dcpu.cpp
    ./dcpu/disassembler.cpp  
    file.c
    )

set(dcpu-vm_SRCS
    dcpu-vm.cpp
    #./dcpu/dcpu.cpp
    #./dcpu/disassembler.cpp  
    binasm.cpp
    #file.c
    )

set(dcpu-devices_SRCS
    ./dcpu/devices/gclock.cpp
    ./dcpu/devices/gkeyboard.cpp
    ./dcpu//devices/lem1802.cpp
    ./dcpu//devices/lem1803.cpp
    ./dcpu//devices/cgm.cpp
    ./dcpu//devices/speaker.cpp
    ./dcpu//devices/m35fd.cpp
    )

set(dcpu-vm_SFML_SRCS
    ./sfml/square_gen.cpp
    ./sfml/KeyboardWindow.cpp
    ./sfml/MonitorWindow.cpp
    )

set(dcpu-vm_GRAPHICS_SRCS
    )

set(TinyXML-2_SRCS
    ./tinyxml2/tinyxml2.cpp
    )

# DCPU-VM core lib
add_library( dcpu
    STATIC
    ${dcpu_SRCS}
    )
    
include_directories(dcpu
    ${DCPU-VM_INCLUDE_DIRS}
)

# DCPU-DEVICES lib
add_library( dcpu-devices
    STATIC 
    ${dcpu-devices_SRCS}
    )
    
include_directories(dcpu-devices
    ${DCPU-VM_INCLUDE_DIRS}
)

# Main executable
add_executable(dcpu-vm 
    ${dcpu-vm_SRCS}
    ${dcpu-vm_GRAPHICS_SRCS}
    ${dcpu-vm_SFML_SRCS}
    ${TinyXML-2_SRCS}
    )

include_directories(dcpu-vm
    ${DCPU-VM_INCLUDE_DIRS}
#    ${OPENGL_INCLUDE_DIR}
    ${SFML_INCLUDE_DIR}
)

target_link_libraries(dcpu-vm
    dcpu
    dcpu-devices
    ${CMAKE_THREAD_LIBS_INIT}
#    ${OPENGL_LIBRARIES}
    ${SFML_LIBRARIES}
)

#millennium 
add_executable( millenium
    millenium/millenium.cpp
    millenium/obj/obj.cpp
    )
    
include_directories(millenium
    ${DCPU-VM_INCLUDE_DIRS}
    ${SFML_INCLUDE_DIR}
)
    
target_link_libraries(millenium
    dcpu
    dcpu-devices
    ${CMAKE_THREAD_LIBS_INIT}
    ${SFML_LIBRARIES}
)
    
# pbm2font tool
add_executable(pbm2font
    pbm2font.cpp
    )

include_directories(pbm2font
    ${DCPU-VM_INCLUDE_DIRS}
)

target_link_libraries(pbm2font
    ${CMAKE_THREAD_LIBS_INIT}
)

# bin2dsk tool
add_executable(bin2dsk
    bin2dsk.cpp
    )

include_directories(bin2dsk
    ${DCPU-VM_INCLUDE_DIRS}
)

target_link_libraries(bin2dsk
    dcpu
    dcpu-devices
    ${CMAKE_THREAD_LIBS_INIT}
)



