set(SSP_LIBRARY_HEADERS
  ${SSP_LIBRARY_HEADERS}
  utils/svg_constants.h
  utils/svg_path_utils.h
  utils/transform_utils.h
  utils/xmlelement.h

  utils/svg_trackconn_util.h

  PARENT_SCOPE
)

set(SSP_LIBRARY_SOURCES
  ${SSP_LIBRARY_SOURCES}
  utils/svg_path_utils.cpp
  utils/transform_utils.cpp
  utils/xmlelement.cpp

  utils/svg_trackconn_util.cpp

  PARENT_SCOPE
)
