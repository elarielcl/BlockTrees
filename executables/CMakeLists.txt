set(project_EXECUTABLES
        main
        padding_main
        inputs
        experiments_block_tree
        experiments_padding_block_tree)


foreach( exe ${project_EXECUTABLES} )
    add_executable( ${exe} ${exe}.cpp)
endforeach()


foreach( exe ${project_EXECUTABLES} )
    target_link_libraries( ${exe} blocktree )
    target_link_libraries( ${exe} ${CMAKE_SOURCE_DIR}/external/sdsl-lite/lib/libsdsl.a )
    target_link_libraries( ${exe} ${CMAKE_SOURCE_DIR}/external/sdsl-lite/lib/libdivsufsort.a )
    target_link_libraries( ${exe} ${CMAKE_SOURCE_DIR}/external/sdsl-lite/lib/libdivsufsort64.a )
endforeach()