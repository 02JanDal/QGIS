set(SURVEY_SRCS
  qgssdalprovider.cpp
  qgssdalconversion.cpp
  qgssdalapproximatingtask.cpp
)
set(SURVEY_HDRS
  qgssdalprovider.h
  qgssdalconversion.h
  qgssdalapproximatingtask.h
)

if (WITH_GUI)
  set(SURVEY_GUI_SRCS
    qgssdalsourceselect.cpp

    qgssdalprovidergui.cpp
  )
  set(SURVEY_GUI_HDRS
    qgssdalsourceselect.h
  )
endif()

########################################################
# Static
add_library(provider_sdal_a STATIC ${SURVEY_SRCS} ${SURVEY_HDRS})

set_target_properties(provider_sdal_a PROPERTIES UNITY_BUILD ${ENABLE_UNITY_BUILDS})

target_include_directories(provider_sdal_a PUBLIC
  ${CMAKE_SOURCE_DIR}/src/providers/survey
)

target_link_libraries(provider_sdal_a
  qgis_core
  sdal
)

# require c++17
target_compile_features(provider_sdal_a PRIVATE cxx_std_17)
target_compile_definitions(provider_sdal_a PRIVATE "-DQT_NO_FOREACH")

if (WITH_GUI)
  qt_wrap_ui(SURVEY_UIS_H ${SURVEY_UIS})

  add_library(provider_sdal_gui_a STATIC ${SURVEY_GUI_SRCS} ${SURVEY_GUI_HDRS} ${SURVEY_UIS_H})

  set_target_properties(provider_sdal_gui_a PROPERTIES UNITY_BUILD ${ENABLE_UNITY_BUILDS})

  target_include_directories(provider_sdal_gui_a PUBLIC
    ${CMAKE_BINARY_DIR}/src/providers/survey
  )

  target_link_libraries(provider_sdal_a
    qgis_gui
  )
  target_link_libraries(provider_sdal_gui_a
    qgis_gui
  )

  # require c++17
  target_compile_features(provider_sdal_gui_a PRIVATE cxx_std_17)
  target_compile_definitions(provider_sdal_gui_a PRIVATE "CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")

  target_compile_definitions(provider_sdal_gui_a PRIVATE "-DQT_NO_FOREACH")

  add_dependencies(provider_sdal_gui_a ui)
endif()

if (FORCE_STATIC_LIBS)
  # for (external) mobile apps to be able to pick up provider for linking
  install (TARGETS provider_sdal_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
  if (WITH_GUI)
    install (TARGETS provider_sdal_gui_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
  endif()
else()
  add_library(provider_sdal MODULE ${SURVEY_SRCS} ${SURVEY_HDRS} ${SURVEY_GUI_SRCS} ${SURVEY_GUI_HDRS} ${SURVEY_UIS_H})

  set_target_properties(provider_sdal PROPERTIES UNITY_BUILD ${ENABLE_UNITY_BUILDS})

  # require c++17
  target_compile_features(provider_sdal PRIVATE cxx_std_17)
  target_compile_definitions(provider_sdal PRIVATE "CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")

  target_compile_definitions(provider_sdal PRIVATE "-DQT_NO_FOREACH")

  target_link_libraries(provider_sdal
    qgis_core
    sdal
  )

  if (WITH_GUI)
    target_link_libraries(provider_sdal
      qgis_gui
    )
    add_dependencies(provider_sdal ui)
  endif()

  # clang-tidy
  if(CLANG_TIDY_EXE)
    set_target_properties(
      provider_sdal PROPERTIES
      CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
    )
  endif()

  install(TARGETS provider_sdal
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
endif()
