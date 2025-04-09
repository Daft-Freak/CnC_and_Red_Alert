add_executable(remix
    main.cpp
    mixwriter.cpp
    names.cpp
    shape.cpp
    xordelta.cpp

    ../../RA/ccfile.cpp
)
target_link_libraries(remix tech)