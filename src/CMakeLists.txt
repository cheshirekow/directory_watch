add_executable(directoryWatch directoryWatch.c)
#target_link_libraries(svg2pdf ${LIBS})
    
install(TARGETS directoryWatch
        RUNTIME DESTINATION bin )
