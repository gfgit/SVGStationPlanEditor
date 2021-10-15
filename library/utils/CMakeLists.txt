set(SSP_LIBRARY_HEADERS
  ${SSP_LIBRARY_HEADERS}
  utils/svg_constants.h
  utils/svg_path_utils.h
  utils/xmlelement.h

  PARENT_SCOPE
)

set(SSP_LIBRARY_SOURCES
  ${SSP_LIBRARY_SOURCES}
  utils/svg_path_utils.cpp
  utils/xmlelement.cpp

  PARENT_SCOPE
)
